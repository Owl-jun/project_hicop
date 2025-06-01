#include "pch.h"
#include "Session.hpp"
#include "SessionManager.hpp"



Session::Session(std::shared_ptr<IOContext> _ctx)
{
	ctx = _ctx;
}

Session::~Session()
{
}

void Session::Recv()
{
	if (ctx->type == TYPE::TYPE) {
		ctx->wsabuf.len = 4;
		ctx->type = TYPE::LENGHTH;
		memcpy(type, ctx->buffer, 1);
		ZeroMemory(&ctx->buffer, sizeof(ctx->buffer));
	}
	else if (ctx->type == TYPE::LENGHTH)
	{
		int val = 0;
		memcpy(&val, ctx->buffer, sizeof(int));
		ntohl(val);
		ctx->wsabuf.len = val;
		
		ctx->type = TYPE::PAYLOAD;
		memcpy(len, ctx->buffer, 4);
		ZeroMemory(&ctx->buffer, sizeof(ctx->buffer));
	}
	else if (ctx->type == TYPE::PAYLOAD)
	{
		memcpy(&payload, ctx->buffer, ctx->wsabuf.len);
		
		ctx->wsabuf.len = 1;
		ctx->type = TYPE::TYPE;
		ZeroMemory(&ctx->buffer, sizeof(ctx->buffer));
	}
	
	ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
	WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
}

void Session::close()
{
	closesocket(ctx->clientSock);
}

void Session::SetPk(int _pk)
{
	pk = _pk;
}

SOCKET Session::GetSocket()
{
	return ctx->clientSock;
}

int Session::GetPK()
{
	return pk;
}

