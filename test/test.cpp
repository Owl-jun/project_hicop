#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    // 소켓 생성 및 설정 (생략)

    if (listenSocket != INVALID_SOCKET && clientSocket == INVALID_SOCKET) {
        int iResult;
        DWORD dwBytes = 0;
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        if (AcceptEx(listenSocket, clientSocket, &clientAddr, &addrLen, &dwBytes, NULL, NULL) == TRUE) {
            printf("AcceptEx 성공!\n");
            // 클라이언트 소켓 사용
        }
        else {
            iResult = WSAGetLastError();
            printf("AcceptEx 실패: %d\n", iResult);
        }
    }

    // 소켓 닫기 및 Winsock 종료 (생략)
    WSACleanup();
    return 0;
}