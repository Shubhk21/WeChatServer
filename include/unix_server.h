#include <iostream>
#include <sys/socket.h>
#include <sys/event.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h> 
#ifdef __APPLE__
    #include <sys/event.h>    
#elif __linux__
    #include <sys/epoll.h> 
#endif
#pragma once

namespace US {
    extern int server_socket;

    extern std::map<std::string, int> usr_to_soc;
    extern std::map<int, std::string> soc_to_usr;

    void createSocket();
    void bindSocket();
    void listenClient();
    int acceptClient();

    void handleSocket();

}   
