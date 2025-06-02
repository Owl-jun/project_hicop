#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#pragma comment(lib,"ws2_32.lib")


char type[1];
char len[4];

void sendmsg(SOCKET consock,std::string,int);
int main() {

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == -1) {
		std::cout << "WSA ERROR" << WSAGetLastError() << std::endl;
	}

	SOCKET consock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (consock == INVALID_SOCKET) {
		std::cerr << "SOCKET MAKE ERROR" << std::endl;
	}

	SOCKADDR_IN address = { 0 };
	address.sin_family = AF_INET;
	address.sin_port = htons(12345);
	inet_pton(AF_INET, "210.119.12.82", &address.sin_addr);

	int result = connect(consock, (sockaddr*)&address, sizeof(address));
	if (result == SOCKET_ERROR) {
		std::cout << "서버 연결 실패!" << WSAGetLastError() << std::endl;
	}

	char buf[1024];
	char sendBuf[1024];
	char recvBuf[2048] = { 0 };
	int recvLenTotal = 0;
	std::string msg;
	std::string id;
	std::cout << "ID 입력 : " << std::endl;
	std::cin >> id;
	sendmsg(consock, id, 2);
	std::chrono::steady_clock::time_point prev;
	std::thread t([&]() {
		while (1)
		{
			int len = recv(consock, buf, 1024, 0);
			if (len > 0) {
				std::string str(buf, len); // 받은 만큼만 문자열로
				auto after = std::chrono::steady_clock::now();
				std::chrono::duration<double> diff = after - prev;
				std::cout << str << std::endl;
				if (str.find(id.c_str(),0) != std::string::npos) {
					std::cout << "경과시간 : " << diff.count() << std::endl;
				}
			}
		}
	});
	while (1) {
		std::getline(std::cin,msg);
		prev = std::chrono::steady_clock::now();
		sendmsg(consock,msg, 1);
	}

	return 0;
}

void sendmsg(SOCKET consock, std::string msg, int id) {
	
	uint32_t payloadLen = htonl(static_cast<uint32_t>(msg.size()));

	char buf[1024] = {};
	if (id == 1) {
		buf[0] = (char)1;
	}
	else if (id == 2) {
		buf[0] = (char)2;
	}
	
	memcpy(buf + 1, &payloadLen, 4);
	memcpy(buf + 5, msg.c_str(), msg.size());

	int totalLen = 5 + msg.size();
	int sent = send(consock, buf, totalLen, 0);
	//std::cout << std::string(buf,totalLen);
	
	if (sent == SOCKET_ERROR) {
		std::cerr << "SEND ERROR" << WSAGetLastError() << std::endl;
	}

}