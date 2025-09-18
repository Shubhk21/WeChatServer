#ifdef _WIN32
#include "config.h"
#include "windows_server.h"

SOCKET WS::server_socket                = INVALID_SOCKET;

HANDLE WS::hiocp;

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

int WS::SendData(std::string data){
    return send(server_socket, data.c_str(), data.length(), 0);
}
#endif


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
            closesocket(client->client_socket);
            delete client;
            continue;
        }

        std::cout << "Received from client: " << client->recvBuff << std::endl;

        ZeroMemory(&client->ol,sizeof(OVERLAPPED));
        DWORD recvflag = 0;
        WSABUF recvBuf = {sizeof(client->recvBuff), client->recvBuff};
        WSARecv(client->client_socket, &recvBuf, 1, NULL, &recvflag, &client->ol, NULL);
    }

    return 0;

}