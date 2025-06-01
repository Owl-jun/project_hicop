#include "pch.h"
#include "CustomStructs.hpp"
#include "WinSockManager.hpp"

WinSockManager& WinSockManager::GetInstance()
{
	static WinSockManager instance;
	return instance;
}

void WinSockManager::Acceptor(short PORT)
{
	// STEP 2. 오버랩I/O를 위한 소켓 생성 및 주소 bind , listen
	listenSock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSock == INVALID_SOCKET) {
		std::cerr << "[FATAL] listenSock 생성 실패: " << WSAGetLastError() << "\n";
	}

	SOCKADDR_IN address = { 0 };
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	if (bind(listenSock, (sockaddr*)&address, sizeof(address)) == -1) {
		std::cout << "Bind Error" << std::endl;
	}
	listen(listenSock, SOMAXCONN);
}

SOCKET WinSockManager::Accept()
{
	SOCKET temp;
	sockaddr_in addr = {0};
	int size = sizeof(addr);
	temp = accept(listenSock, (sockaddr*)&addr, &size);
	return temp;
}

void WinSockManager::Async_accept()
{
	auto* acceptCtx = new AcceptContext;
	acceptCtx->oper = OPER::ACCEPT;
	acceptCtx->clientSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (acceptCtx->clientSock == INVALID_SOCKET) { 
		std::cerr << "[FATAL] acceptSock 생성 실패: " << WSAGetLastError() << "\n"; 
	}

	HANDLE h = CreateIoCompletionPort((HANDLE)acceptCtx->clientSock, iocp, (ULONG_PTR)acceptCtx->clientSock, 0);
	if (h == NULL) {
		std::cerr << "[FATAL] acceptSock IOCP 등록 실패: " << GetLastError() << "\n";
	}

	loadAcceptEx();

	BOOL result = lpAcceptEx(listenSock, acceptCtx->clientSock, acceptCtx->buffer, 0,
		ADDR_BUF_SIZE, ADDR_BUF_SIZE, NULL, &acceptCtx->overlapped);
	if (!result && WSAGetLastError() != ERROR_IO_PENDING) {
		std::cerr << "[ERROR] main() : AcceptEx failed: " << WSAGetLastError() << "\n";
	}

}

void WinSockManager::Async_accept(SOCKET key)
{
	auto* newCtx = new AcceptContext;
	newCtx->oper = OPER::ACCEPT;
	newCtx->clientSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort((HANDLE)newCtx->clientSock, iocp, (ULONG_PTR)newCtx->clientSock, 0);

	BOOL result = lpAcceptEx((SOCKET)key, newCtx->clientSock, newCtx->buffer, 0,
		ADDR_BUF_SIZE, ADDR_BUF_SIZE, NULL, &newCtx->overlapped
	);
	if (!result && WSAGetLastError() != ERROR_IO_PENDING) {
		std::cerr << "[ERROR] AcceptEx failed: " << WSAGetLastError() << "\n";
	}
}

void WinSockManager::Accept_Mode(MODE a)
{
	if (a == MODE::ASYNC) {
		iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (!CreateIoCompletionPort((HANDLE)listenSock, iocp, (ULONG_PTR)listenSock, 0)) {
			std::cerr << "[FATAL] listenSock IOCP 등록 실패: " << GetLastError() << "\n";
		}
	}
}

HANDLE WinSockManager::GetIocp()
{
	return iocp;
}

WinSockManager::WinSockManager()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == -1) {
		std::cout << "WSAStartup Error" << std::endl;
	}
}

void WinSockManager::loadAcceptEx() {
	GUID guid = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	// WSAIoctl , AcceptEx 함수는 확장 함수이기 때문에 OS에게 함수주소 보내달라고 요청
	// guid 에 해당하는 함수포인터를 lpAcceptEx 에 넣어달라
	// SIO_GET_EXTENSION_FUNCTION_POINTER 확장 함수임을 표현
	int res = WSAIoctl(
		listenSock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid),
		&lpAcceptEx, sizeof(lpAcceptEx),
		&bytes, nullptr, nullptr
	);
	if (res == SOCKET_ERROR) {
		std::cerr << "[FATAL] WSAIoctl failed: " << WSAGetLastError() << "\n";
	}
	if (lpAcceptEx == nullptr) { std::cout << "함수 로드실패" << std::endl; }

	// 이 함수 호출 후 lpAcceptEx 는 AcceptEx 함수를 가르키는 포인터임
}
WinSockManager::~WinSockManager()
{
	closesocket(listenSock);
	WSACleanup();
}
