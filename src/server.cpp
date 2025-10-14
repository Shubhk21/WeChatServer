#include<iostream>
#include <thread>
#include "client_auth.h"

#ifdef _WIN32
#include "windows_server.h"
#include "client_context.h"
std::vector<ClientContext *> clients;
#elif defined(__APPLE__) || defined(__linux__)
#include "unix_server.h"
#endif


void chatServer(){

    #ifdef _WIN32

    WS::hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);


    for(int tcount=0;tcount<8;tcount++){
        CreateThread(nullptr,0,WS::WorkerThread,nullptr,0,nullptr);
    }

    WS::Initialize();
    WS::CreateSocket();
    WS::Bind();
    WS::Listen();
    
    while(true){
        SOCKET client_socket = WS::AcceptSocket();
        std::cout << "Chat started!" << std::endl;
        ClientContext * client  = new ClientContext();
        client->client_socket = client_socket;

        CreateIoCompletionPort((HANDLE)client_socket,WS::hiocp,(ULONG_PTR)client,0);

        ZeroMemory(&client->ol,sizeof(OVERLAPPED));
        DWORD recvFlags = 0;
        WSABUF recvBuf = { sizeof(client->recvBuff), client->recvBuff };
        WSARecv(client_socket,&recvBuf,1,nullptr,&recvFlags,&client->ol,nullptr);
        clients.push_back(client);
    }

    closesocket(WS::server_socket);
    WSACleanup();

    #elif defined(__APPLE__) || defined(__linux__)
    US::createSocket();
    US::bindSocket();
    US::listenClient();
    US::handleSocket();
    #endif

}


int main(){

    try
    {
        std::cout<<"Server Started!"<<std::endl; 

        std::thread auth_server_thread(handleClientAuth);

        std::thread chat_server_thread(chatServer);

        auth_server_thread.join();

        chat_server_thread.join();

        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(-1);
    }
    
}