#include <winsock2.h>
#include <mswsock.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

const int PORT = 12345;
const int ADDR_BUF_SIZE = sizeof(sockaddr_in) + 16;

LPFN_ACCEPTEX lpAcceptEx = nullptr;

void LoadAcceptEx(SOCKET listenSock) {
    GUID guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guid, sizeof(guid),
        &lpAcceptEx, sizeof(lpAcceptEx),
        &bytes, nullptr, nullptr);
}

struct AcceptContext {
    OVERLAPPED overlapped = {};
    SOCKET acceptSock = INVALID_SOCKET;
    char buffer[ADDR_BUF_SIZE * 2 + 32] = {};
};

void WorkerThread(HANDLE iocp) {
    DWORD transferred;
    ULONG_PTR key;
    LPOVERLAPPED ov;

    while (true) {
        BOOL ok = GetQueuedCompletionStatus(iocp, &transferred, &key, &ov, INFINITE);
        if (!ok || ov == nullptr) {
            std::cerr << "[!] Accept failed or server closed\n";
            continue;
        }

        auto* ctx = reinterpret_cast<AcceptContext*>(ov);
        std::cout << "[+] Client connected!\n";

        closesocket(ctx->acceptSock);  // 이번엔 연결 확인만 하고 종료
        delete ctx;

        // 다음 Accept 예약
        auto* newCtx = new AcceptContext;
        newCtx->acceptSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        lpAcceptEx((SOCKET)key, newCtx->acceptSock, newCtx->buffer, 0,
            ADDR_BUF_SIZE, ADDR_BUF_SIZE, NULL, &newCtx->overlapped);
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, SOMAXCONN);

    LoadAcceptEx(listenSock);

    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort((HANDLE)listenSock, iocp, (ULONG_PTR)listenSock, 0);

    std::thread(WorkerThread, iocp).detach();

    // 첫 Accept 예약
    auto* ctx = new AcceptContext;
    ctx->acceptSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    lpAcceptEx(listenSock, ctx->acceptSock, ctx->buffer, 0,
        ADDR_BUF_SIZE, ADDR_BUF_SIZE, NULL, &ctx->overlapped);

    std::cout << "[*] Server running on port " << PORT << "\n";
    std::cin.get();  // enter 입력시 종료

    closesocket(listenSock);
    WSACleanup();
    return 0;
}
