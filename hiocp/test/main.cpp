// #include <hiocp.hpp>


#include <iostream>
#include <thread>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

const int PORT = 12345;
const int ADDR_BUF_SIZE = sizeof(sockaddr_in) + 16;

LPFN_ACCEPTEX lpAcceptEx = nullptr;

void LoadAcceptEx(SOCKET listenSock) {
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
	// 이 함수 호출 후 lpAcceptEx 는 AcceptEx 함수를 가르키는 포인터임
}

enum class OPER { ACCEPT, RECV, SEND };

struct BaseContext {
	OVERLAPPED overlapped = {};
	OPER oper;
	SOCKET clientSock = INVALID_SOCKET;
};

struct IOContext : BaseContext{
	WSABUF wsabuf = {};
	char buffer[4096] = {};
	DWORD BUFSIZE = sizeof(buffer);
	DWORD flags = 0;

};

struct AcceptContext : BaseContext {
	char buffer[ADDR_BUF_SIZE * 2 + 32] = {};
	DWORD flags = 0;

};

void HandleAccept(AcceptContext* ctx) {
	std::cout << "[+] Client connected!\n";

	auto* cliCtx = new IOContext;
	cliCtx->clientSock = ctx->clientSock;
	cliCtx->wsabuf.buf = cliCtx->buffer;
	cliCtx->wsabuf.len = sizeof(cliCtx->buffer);
	cliCtx->oper = OPER::RECV;
	WSARecv(cliCtx->clientSock, &cliCtx->wsabuf, 1, nullptr, &ctx->flags, &cliCtx->overlapped, nullptr);

}

void HandleRecv(IOContext* ctx, size_t transferred) {
	std::string data(ctx->buffer, transferred);
	std::cout << "Received : " << data << std::endl;
	ctx->wsabuf.len = strlen(ctx->buffer);
	ctx->oper = OPER::SEND;
	ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
	WSASend(ctx->clientSock, &ctx->wsabuf, 1,nullptr, 0, &ctx->overlapped, nullptr);
}

void HandleSend(IOContext* ctx) {
	std::cout << "Send Success " << std::endl;
	ctx->wsabuf.len = sizeof(ctx->buffer);
	ctx->oper = OPER::RECV;
	ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
	WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
}

void WorkerThread(HANDLE iocp) {
	DWORD transferred;
	ULONG_PTR key;
	LPOVERLAPPED ov;

	while (true) {
		BOOL ok = GetQueuedCompletionStatus(iocp, &transferred, &key, &ov, INFINITE);
		if (ov == nullptr) {
			std::cerr << "[!] GQCS failed or shutdown\n";
			continue;
		}

		auto* ctx = reinterpret_cast<BaseContext*>(ov);

		if (!ok && transferred == 0) {
			std::cerr << "[!] Client forcibly disconnected or error\n";

			if (ctx->oper == OPER::ACCEPT) {
				closesocket(ctx->clientSock);
				delete reinterpret_cast<AcceptContext*>(ctx);
			}
			else {
				auto* ioCtx = reinterpret_cast<IOContext*>(ctx);
				closesocket(ioCtx->clientSock);
				delete ioCtx;
			}
			continue;
		}
		
		// 연결 처리
		if (ctx->oper == OPER::ACCEPT) {

			HandleAccept(reinterpret_cast<AcceptContext*>(ctx));
			delete reinterpret_cast<AcceptContext*>(ctx);

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
		else {
			// 게이트웨이
			switch (ctx->oper) {
			case OPER::RECV:
				HandleRecv(reinterpret_cast<IOContext*>(ctx),transferred);
				break;
			case OPER::SEND:
				HandleSend(reinterpret_cast<IOContext*>(ctx));
				break;
			}
		}
	}
}

SOCKET listenSock = INVALID_SOCKET;
int main() {

	// STEP 1. 네트워크 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == -1) {
		std::cout << "WSAStartup Error" << std::endl;
	}

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

	// CP 생성 및 소켓 할당
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!CreateIoCompletionPort((HANDLE)listenSock, iocp, (ULONG_PTR)listenSock, 0)) {
		std::cerr << "[FATAL] listenSock IOCP 등록 실패: " << GetLastError() << "\n";
	}

	// STEP 3. AcceptEx 함수포인터 호출
	LoadAcceptEx(listenSock);
	if (lpAcceptEx == nullptr) { std::cout << "함수 로드실패" << std::endl; }

	std::thread(WorkerThread, iocp).detach();

	// 첫 Accept 예약
	auto* ctx = new AcceptContext;
	ctx->oper = OPER::ACCEPT;
	ctx->clientSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ctx->clientSock == INVALID_SOCKET) {
		std::cerr << "[FATAL] acceptSock 생성 실패: " << WSAGetLastError() << "\n";
	}

	HANDLE h = CreateIoCompletionPort((HANDLE)ctx->clientSock, iocp, (ULONG_PTR)ctx->clientSock, 0);
	if (h == NULL) {
		std::cerr << "[FATAL] acceptSock IOCP 등록 실패: " << GetLastError() << "\n";
	}

	BOOL result = lpAcceptEx(listenSock, ctx->clientSock, ctx->buffer, 0,
		ADDR_BUF_SIZE, ADDR_BUF_SIZE, NULL, &ctx->overlapped);
	if (!result && WSAGetLastError() != ERROR_IO_PENDING) {
		std::cerr << "[ERROR] main() : AcceptEx failed: " << WSAGetLastError() << "\n";
	}

	std::cout << "[*] Server running on port " << PORT << "\n";





	std::cin.get();  // enter 입력시 종료

	closesocket(listenSock);
	WSACleanup();
	return 0;
}