// #include <hiocp.hpp>


#include <iostream>
#include <thread>


#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

int main() {


	GUID guidAcceptEx = WSAID_ACCEPTEX;

	SOCKET listenSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN adress = { 0 };
	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = htonl(INADDR_ANY);
	adress.sin_port = htons(9000);

	bind(listenSock, (sockaddr*)&adress, sizeof(adress));
	listen(listenSock, SOMAXCONN);

	char listenBuf[512];
	OVERLAPPED overlapped = {};
	WSABUF wsabuf = { sizeof(listenBuf), listenBuf };

	// AcceptEx(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER,)  さごごごごごげ
}