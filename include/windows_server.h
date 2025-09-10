#pragma once
#include<iostream>
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<tchar.h>
#pragma comment(lib,"ws2_32.lib")

namespace WS{

    extern SOCKET server_socket;

    void Initialize(); // initialize winsock

    void CreateSocket(); // create socket

    void Bind(); // bind server address

    void Listen(); // listen to clients

    SOCKET AcceptSocket();

    int SendData(std::string data); // send data
}