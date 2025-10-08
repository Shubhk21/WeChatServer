#ifdef _WIN32
#include "config.h"
#include "windows_server.h"

SOCKET WS::server_socket = INVALID_SOCKET;

HANDLE WS::hiocp;

std::map<std::string, SOCKET> WS::usr_to_soc;

std::map<SOCKET, std::string> WS::soc_to_usr;

void WS::Initialize()
{

    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        std::cerr << "Failed to create socket address!";
        exit(-1);
    }
}

void WS::CreateSocket()
{
    try
    {
        server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(-1);
    }
    
}

void WS::Bind()
{
    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CONFIG::SERVER_PORT_SOCKET);

    if (InetPtonA(AF_INET, CONFIG::SERVER_IP.c_str(), &server_address.sin_addr) != 1)
    {
        std::cerr << "Failed to create socket address!";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }

    if (bind(server_socket, reinterpret_cast<SOCKADDR *>(&server_address), sizeof(sockaddr)) != 0)
    {
        std::cerr << "Binding failed";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }
}

void WS::Listen()
{
    if (listen(server_socket, SOMAXCONN) != 0)
    {
        std::cerr << "Listening failed";
        closesocket(server_socket);
        WSACleanup();
        exit(-1);
    }
}

SOCKET WS::AcceptSocket()
{
    return accept(server_socket, nullptr, nullptr);
}

int WS::SendData(std::string sender, std::string data, SOCKET client_socket)
{
    nlohmann::json data_packet = {
        {"sender", sender},
        {"data", data}};

    return send(client_socket, data_packet.dump().c_str(), data_packet.dump().length(), 0);
}

DWORD WINAPI WS::WorkerThread(LPVOID lpParam)
{
    while (1)
    {
        try
        {
            DWORD bytesTransferred;
            ULONG_PTR completionKey;
            OVERLAPPED *pOv;

            BOOL ok = GetQueuedCompletionStatus(hiocp, &bytesTransferred, &completionKey, &pOv, INFINITE);

            ClientContext *client = (ClientContext *)completionKey;

            if (!ok || bytesTransferred == 0)
            {
                std::cerr << "Client disconnected! : " << GetLastError() << '\n';
                WS::usr_to_soc.erase(WS::soc_to_usr[client->client_socket]);
                WS::soc_to_usr.erase(client->client_socket);
                std::cout << WS::soc_to_usr.size() << " " << WS::usr_to_soc.size() << std::endl;
                closesocket(client->client_socket);
                delete client;
                continue;
            }

            std::cout << "Received from client: " << client->recvBuff << " " << bytesTransferred << std::endl;
            std::string send_user, data_to_send, recieve_user;
            try
            {

                auto j_data = nlohmann::json::parse(client->recvBuff);

                send_user = j_data.at("sender").get<std::string>();
                data_to_send = j_data.at("data").get<std::string>();
                recieve_user = j_data.at("receiver").get<std::string>();
            }

            catch (const std::exception &e)
            {
                ZeroMemory(&client->ol, sizeof(OVERLAPPED));
                DWORD recvflag = 0;
                ZeroMemory(&client->recvBuff, sizeof(client->recvBuff));
                WSABUF recvBuf = {sizeof(client->recvBuff), client->recvBuff};
                WSARecv(client->client_socket, &recvBuf, 1, NULL, &recvflag, &client->ol, NULL);
                continue;
            }

            if (data_to_send == "hand_shake" && recieve_user == "hand_shake")
            {
                WS::usr_to_soc.emplace(send_user, client->client_socket);
                WS::soc_to_usr.emplace(client->client_socket, send_user);
                std::cout << "hand shake\n";
            }
            else
            {

                auto find_user = WS::usr_to_soc.find(recieve_user);

                if (find_user != WS::usr_to_soc.end())
                {
                    int bytes_send = WS::SendData(send_user, data_to_send, find_user->second);
                }
                else
                {
                    WS::SendData("Server", "User Offline", client->client_socket);
                }
            }

            ZeroMemory(&client->ol, sizeof(OVERLAPPED));
            DWORD recvflag = 0;
            ZeroMemory(&client->recvBuff, sizeof(client->recvBuff));
            WSABUF recvBuf = {sizeof(client->recvBuff), client->recvBuff};
            WSARecv(client->client_socket, &recvBuf, 1, NULL, &recvflag, &client->ol, NULL);
        }
        catch (const std::exception &e)
        {
            std::cout << "LOL: " << e.what() << std::endl;
            continue;
        }
    }

    return 0;
}

#endif