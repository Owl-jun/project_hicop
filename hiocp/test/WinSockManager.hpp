#pragma once
#include <Windows.h>

enum class MODE {
	SYN,
	ASYNC
};

class WinSockManager
{
public:
	static WinSockManager& GetInstance();
	void Acceptor(short PORT);
	SOCKET Accept();
	void Async_accept();
	void Async_accept(SOCKET key);
	void Accept_Mode(MODE a);
	HANDLE GetIocp();
	
private:
	WinSockManager();
	~WinSockManager();
	void loadAcceptEx();
	LPFN_ACCEPTEX lpAcceptEx;
	SOCKET listenSock;
	HANDLE iocp;
};

