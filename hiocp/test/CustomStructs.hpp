#pragma once
#include "pch.h"


enum class OPER { ACCEPT, RECV, SEND };
enum class TYPE { TYPE
	, LENGHTH
	, PAYLOAD
	, FINISH };
const int ADDR_BUF_SIZE = sizeof(sockaddr_in) + 16;



struct BaseContext {
	OVERLAPPED overlapped = {};
	OPER oper;
	TYPE type = TYPE::TYPE;
	SOCKET clientSock = INVALID_SOCKET;
};

struct IOContext : BaseContext {
	WSABUF wsabuf = {};
	char buffer[4096] = {};
	DWORD BUFSIZE = sizeof(buffer);
	DWORD flags = 0;

};

struct AcceptContext : BaseContext {
	char buffer[ADDR_BUF_SIZE * 2 + 32] = {};
	DWORD flags = 0;

};