#pragma once
#include <WinSock2.h>


struct ClientContext
{
    SOCKET client_socket;
    char recvBuff[1024];
    OVERLAPPED ol;
};
