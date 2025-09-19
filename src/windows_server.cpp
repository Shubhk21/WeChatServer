#ifdef _WIN32
#include "config.h"
#include "windows_server.h"

SOCKET WS::server_socket                = INVALID_SOCKET;

HANDLE WS::hiocp;

std::map<std::string,SOCKET> WS::usr_to_soc;

std::map<SOCKET,std::string> WS::soc_to_usr;

void WS::Initialize(){

    WSADATA wsadata;

    if(WSAStartup(MAKEWORD(2, 2), &wsadata) !=0){
        std::cerr << "Failed to create socket address!";
        exit(-1);
    }
}


void WS::CreateSocket(){

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

}

void WS::Bind(){
    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CONFIG::SERVER_PORT_SOCKET);

    if (InetPtonA(AF_INET, CONFIG::SERVER_IP.c_str(), &server_address.sin_addr) != 1) {
        std::cerr << "Failed to create socket address!";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }

    if (bind(server_socket, reinterpret_cast<SOCKADDR*>(&server_address), sizeof(sockaddr)) != 0) {
        std::cerr << "Binding failed";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }
}

void WS::Listen(){
    if (listen(server_socket, SOMAXCONN) !=0) {
        std::cerr << "Listening failed";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }
}


SOCKET WS::AcceptSocket(){
    return accept(server_socket, nullptr, nullptr);
}

int WS::SendData(std::string sender,std::string data,SOCKET client_socket){
    std::cout<<"inside senddatta\n";
    nlohmann::json data_packet = {
        {"sender" , sender},
        {"data" , data}
    };

    std::cout<<sender<<" "<<data<<std::endl;
    return send(client_socket, data_packet.dump().c_str(), data_packet.dump().length(), 0);
}


DWORD WINAPI WS::WorkerThread(LPVOID lpParam){

    while(1){
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        OVERLAPPED* pOv;

        BOOL ok = GetQueuedCompletionStatus(hiocp, &bytesTransferred, &completionKey, &pOv, INFINITE);

        if(!ok){
            std::cerr<<"Error occured in worker thread Queue function!\n";
            continue;
        }
        ClientContext* client = (ClientContext*)completionKey;

        if(bytesTransferred==0){
            std::cout<<"Client disconnected!\n";
            WS::usr_to_soc.erase(WS::soc_to_usr[client->client_socket]);
            WS::soc_to_usr.erase(client->client_socket);
            closesocket(client->client_socket);
            delete client;
            continue;
        }

        std::cout << "Received from client: " << client->recvBuff << std::endl;

        auto j_data = nlohmann::json::parse(client->recvBuff);

        std::string send_user = j_data.at("sender").get<std::string>();
        std::string data_to_send = j_data.at("data").get<std::string>();
        std::string recieve_user = j_data.at("reciever").get<std::string>();


        if(data_to_send=="hand_shake" && recieve_user=="hand_shake"){
            WS::usr_to_soc.emplace(send_user,client->client_socket);
            WS::soc_to_usr.emplace(client->client_socket,send_user);
            std::cout<<"hand shake\n";
        }
        else{

            std::cout<<"message to send\n";

            auto find_user  = WS::usr_to_soc.find(recieve_user);

            if(find_user!=WS::usr_to_soc.end()){
                std::cout<<"sending message\n";
                int bytes_send = WS::SendData(send_user,data_to_send,find_user->second);
                std::cout<<"bytes :"<<" "<<bytes_send<<'\n';
            }
            else{
                std::cout<<"No such User\n";
                WS::SendData("","No such User!",client->client_socket);
            }
        }


        ZeroMemory(&client->ol,sizeof(OVERLAPPED));
        DWORD recvflag = 0;
        ZeroMemory(&client->recvBuff,sizeof(client->recvBuff));
        WSABUF recvBuf = {sizeof(client->recvBuff), client->recvBuff};
        WSARecv(client->client_socket, &recvBuf, 1, NULL, &recvflag, &client->ol, NULL);
    }

    return 0;

}

#endif