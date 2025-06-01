#include "pch.h"
#include "Session.hpp"
#include "SessionManager.hpp"
#include "LogicWorker.hpp"


Session::Session(std::shared_ptr<IOContext> _ctx)
{
	ctx = _ctx;
}

Session::~Session()
{
}

void Session::Recv(size_t transferred)
{
	if (ctx->type == TYPE::TYPE) {
		memcpy(type + typerecvbytes, ctx->buffer, transferred);
		typerecvbytes += transferred;

		if (typerecvbytes >= 1) {
			ctx->wsabuf.len = 4;
			ctx->type = TYPE::LENGHTH;
			isEnd = true;
			typerecvbytes = 0;
		}
		else
		{
			isEnd = false;
			ctx->wsabuf.len = 1;
			ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
			WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
		}
	}
	else if (ctx->type == TYPE::LENGHTH)
	{
		memcpy(len + lenrecvbytes, ctx->buffer, transferred);
		lenrecvbytes += transferred;
		
		if (lenrecvbytes >= 4) {
			int val = 0;
			memcpy(&val, len, 4);
			val = ntohl(val);
			packetLength = val;
			ctx->wsabuf.len = val;

			payload.clear();
			ctx->type = TYPE::PAYLOAD;
			lenrecvbytes = 0;
			isEnd = true;
		}
		else {
			isEnd = false;
			ctx->wsabuf.len = 4 - lenrecvbytes;
			ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
			WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
		}
	}
	else if (ctx->type == TYPE::PAYLOAD)
	{
		payload.append(ctx->buffer, transferred);
		plrecvbytes += transferred;
		
		if (plrecvbytes >= packetLength)
		{
			ctx->wsabuf.len = 1;
			ctx->type = TYPE::TYPE;
			Task t = { shared_from_this(),type[0],payload };
			LogicWorker::GetInstance().PustTask(t);
			plrecvbytes = 0;
			isEnd = true;
		}
		else {
			isEnd = false;
			ctx->wsabuf.len = packetLength - plrecvbytes;
			ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
			WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
		}
	}
	if (isEnd) {
		ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
		WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
	}
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


