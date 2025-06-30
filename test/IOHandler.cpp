#include "pch.h"
#include "IOHandler.hpp"
#include "CustomStructs.hpp"
#include "SessionManager.hpp"
#include "Session.hpp"
#include "LogicWorker.hpp"
#include "timetest.hpp"
void IOHandler::OnAccept(AcceptContext* ctx, size_t transferred)
{
	std::cout << "[+] Client connected!\n";

	std::shared_ptr<IOContext> cliCtx = std::make_shared<IOContext>();
	cliCtx->clientSock = ctx->clientSock;
	cliCtx->wsabuf.buf = cliCtx->buffer;
	cliCtx->wsabuf.len = 0;
	cliCtx->oper = OPER::RECV;
	auto session(std::make_shared<Session>(cliCtx));
	SessionManager::GetInstance().createSession(session);
	WSARecv(cliCtx->clientSock, &cliCtx->wsabuf, 1, nullptr, &cliCtx->flags, &cliCtx->overlapped, nullptr);
}

void IOHandler::OnRecv(IOContext* ctx, size_t transferred)
{
	int pk = SessionManager::GetInstance().FindBySocket(ctx->clientSock);
	auto session = SessionManager::GetInstance().GetSessions()[pk];
	session->Recv(transferred);
}

void IOHandler::OnSend(IOContext* ctx)
{
	//timetest::GetInstance().stop();
	std::cout << "current Session Count : " << SessionManager::GetInstance().GetSessions().size() << std::endl;
	ctx->oper = OPER::RECV;
}
