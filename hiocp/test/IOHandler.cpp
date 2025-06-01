#include "pch.h"
#include "IOHandler.hpp"
#include "CustomStructs.hpp"
#include "SessionManager.hpp"
#include "Session.hpp"
#include "LogicWorker.hpp"

void IOHandler::OnAccept(AcceptContext* ctx, size_t transferred)
{
	std::cout << "[+] Client connected!\n";

	std::shared_ptr<IOContext> cliCtx = std::make_shared<IOContext>();
	cliCtx->clientSock = ctx->clientSock;
	cliCtx->wsabuf.buf = cliCtx->buffer;
	cliCtx->oper = OPER::RECV;
	auto session(std::make_shared<Session>(cliCtx));
	SessionManager::GetInstance().createSession(session);
	session->Recv(transferred);
}

void IOHandler::OnRecv(IOContext* ctx, size_t transferred)
{
	int pk = SessionManager::GetInstance().FindBySocket(ctx->clientSock);
	auto session = SessionManager::GetInstance().GetSessions()[pk];
	session->Recv(transferred);
}

void IOHandler::OnSend(IOContext* ctx)
{
	std::cout << "Send Success " << std::endl;
	ctx->wsabuf.len = sizeof(ctx->buffer);
	ctx->oper = OPER::RECV;
	ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
	WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
}
