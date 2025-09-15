#ifdef __APPLE__
#include "config.h"
#include "unix_server.h"

int US::server_socket;

void US::createSocket(){
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){
        std::cerr << "Error: Creating socket \n";
        exit(-1);
    }

    std::cout<<"Server Socket is created! \n";
}

void US::bindSocket(){
    sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(CONFIG::SERVER_PORT);

    if(inet_pton(AF_INET, CONFIG::SERVER_IP.c_str(), &server_info.sin_addr.s_addr) != 1){   //Error case
        std::cerr << "Error: Server configuration \n";
        close(server_socket);
        return;
    }

    if(bind(server_socket, (struct sockaddr*)&server_info, sizeof(server_info)) != 0){
        std::cerr << "Binding failed \n";
        close(server_socket);
        return;
    }

    std::cout<<"Server configured! \n";
}

void US::listenClient(){
    if(listen(server_socket, SOMAXCONN) != 0){
        std::cerr << "Listening failed \n";
        close(server_socket);
        return;
    }
}

int US::acceptClient(){
    return accept(server_socket, nullptr, nullptr);
}

#endif
