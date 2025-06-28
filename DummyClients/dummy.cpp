#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#pragma comment(lib,"ws2_32.lib")


char type[1];
char len[4];

int main() {

	int num;
	std::cout << "더미 클라이언트 생성개수 : " << std::endl;
	std::cin >> num;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == -1) {
		std::cout << "WSA ERROR" << WSAGetLastError() << std::endl;
	}

	for (int i = 0; i < num; ++i) {
		std::thread([&]() {

			SOCKET consock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (consock == INVALID_SOCKET) {
				std::cerr << "SOCKET MAKE ERROR" << std::endl;
			}

			SOCKADDR_IN address = { 0 };
			address.sin_family = AF_INET;
			address.sin_port = htons(12345);
			inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

			int result = connect(consock, (sockaddr*)&address, sizeof(address));
			if (result == SOCKET_ERROR) {
				std::cout << "서버 연결 실패!" << WSAGetLastError() << std::endl;
			}
			while (1) {
				getchar();
			}
		}).detach();
	}
	std::cout << num << "개 연결완료." << std::endl;
	std::cout << "종료 : q" << std::endl;
	bool run = true;
	while (run) {
		char a = getchar();
		if (a == 'q') { run = false; }
	}

	return 0;
}
