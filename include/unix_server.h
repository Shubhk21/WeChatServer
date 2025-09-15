#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h> 
#pragma once

namespace US {
    extern int server_socket;

    void createSocket();
    void bindSocket();
    void listenClient();
    int acceptClient();
}   
