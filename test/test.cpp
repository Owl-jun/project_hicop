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

    // ���� ���� �� ���� (����)

    if (listenSocket != INVALID_SOCKET && clientSocket == INVALID_SOCKET) {
        int iResult;
        DWORD dwBytes = 0;
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        if (AcceptEx(listenSocket, clientSocket, &clientAddr, &addrLen, &dwBytes, NULL, NULL) == TRUE) {
            printf("AcceptEx ����!\n");
            // Ŭ���̾�Ʈ ���� ���
        }
        else {
            iResult = WSAGetLastError();
            printf("AcceptEx ����: %d\n", iResult);
        }
    }

    // ���� �ݱ� �� Winsock ���� (����)
    WSACleanup();
    return 0;
}