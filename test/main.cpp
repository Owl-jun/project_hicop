#include "pch.h"
#include "utils.hpp"
#include "CustomStructs.hpp"
#include "WinSockManager.hpp"
#include "Session.hpp"
#include "SessionManager.hpp"
#include "Server.hpp"
#include "LogicWorker.hpp"



const short PORT = fileutils::getPortByJson();
LPFN_ACCEPTEX lpAcceptEx = nullptr;

int main() {
	SetConsoleOutputCP(CP_UTF8);

	WinSockManager& wsm = WinSockManager::GetInstance();
	wsm.Acceptor(PORT);
	wsm.Accept_Mode(MODE::ASYNC);
	wsm.Async_accept();

	Server server;
	LogicWorker::GetInstance().Run();
	server.Run();
	
	std::cout << "[*] Server running on port " << PORT << "\n";

	return 0;
}