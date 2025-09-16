#include<iostream>
#include <thread>
#include "client_auth.h"

#ifdef _WIN32
#include "windows_server.h"


#elif APPLE

#endif


void chatServer(){

    #ifdef _WIN32

    WS::Initialize();
    WS::CreateSocket();
    WS::Bind();
    WS::Listen();
    
    while(true){
        SOCKET server_chat_sopy = WS::AcceptSocket();
        std::cout << "Chat started!" << std::endl;
    }

    #elif APPLE

    #endif

}

int main(){

    std::cout<<"Server Started!"<<std::endl; 

    std::thread auth_server_thread(handleClientAuth);

    // std::thread chat_server_thread(chatServer);

    auth_server_thread.join();

    // chat_server_thread.join();

    return 0;
}