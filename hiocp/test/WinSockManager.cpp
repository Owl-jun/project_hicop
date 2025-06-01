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
	// STEP 2. ������I/O�� ���� ���� ���� �� �ּ� bind , listen
	listenSock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSock == INVALID_SOCKET) {
		std::cerr << "[FATAL] listenSock ���� ����: " << WSAGetLastError() << "\n";
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
		std::cerr << "[FATAL] acceptSock ���� ����: " << WSAGetLastError() << "\n"; 
	}

	HANDLE h = CreateIoCompletionPort((HANDLE)acceptCtx->clientSock, iocp, (ULONG_PTR)acceptCtx->clientSock, 0);
	if (h == NULL) {
		std::cerr << "[FATAL] acceptSock IOCP ��� ����: " << GetLastError() << "\n";
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
			std::cerr << "[FATAL] listenSock IOCP ��� ����: " << GetLastError() << "\n";
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

	// WSAIoctl , AcceptEx �Լ��� Ȯ�� �Լ��̱� ������ OS���� �Լ��ּ� �����޶�� ��û
	// guid �� �ش��ϴ� �Լ������͸� lpAcceptEx �� �־�޶�
	// SIO_GET_EXTENSION_FUNCTION_POINTER Ȯ�� �Լ����� ǥ��
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
	if (lpAcceptEx == nullptr) { std::cout << "�Լ� �ε����" << std::endl; }

	// �� �Լ� ȣ�� �� lpAcceptEx �� AcceptEx �Լ��� ����Ű�� ��������
}
WinSockManager::~WinSockManager()
{
	closesocket(listenSock);
	WSACleanup();
}
