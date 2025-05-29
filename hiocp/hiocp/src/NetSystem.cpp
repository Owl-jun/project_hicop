#include "pch.h"
#include "NetSystem.hpp"


hiocp::NetSystem::NetSystem()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

hiocp::NetSystem::~NetSystem()
{
    WSACleanup();
}
