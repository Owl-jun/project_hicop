#include "pch.h"
#include "utils.hpp"
#include "Server.hpp"
#include "CustomStructs.hpp"
#include "WinSockManager.hpp"
#include "SessionManager.hpp"
#include "Session.hpp"
#include "IOHandler.hpp"


void Server::Thread_Work(HANDLE iocp)
{
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
				auto pk = SessionManager::GetInstance().FindBySocket(ctx->clientSock);
				SessionManager::GetInstance().closeSession(pk);
			}
			continue;
		}

		// 연결 처리
		if (ctx->oper == OPER::ACCEPT) {
			IOHandler::OnAccept(reinterpret_cast<AcceptContext*>(ctx, transferred));
			delete reinterpret_cast<AcceptContext*>(ctx);
			WinSockManager::GetInstance().Async_accept(key);
		}
		else {
			// 게이트웨이
			switch (ctx->oper) {
			case OPER::RECV:
				IOHandler::OnRecv(reinterpret_cast<IOContext*>(ctx), transferred);
				break;
			case OPER::SEND:
				IOHandler::OnSend(reinterpret_cast<IOContext*>(ctx));
				break;
			}
		}
	}
}

Server::Server()
{
}

Server::~Server()
{
	for (int i = 0; i < 4; ++i) {
		IOPool[i].join();
	}
}

void Server::Run()
{
	for (int i = 0; i < 4; ++i) {
		IOPool.push_back(std::thread([this]() {
			Thread_Work(WinSockManager::GetInstance().GetIocp());
			}));
	}
}

