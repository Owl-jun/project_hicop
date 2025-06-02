



/////

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

void Session::pushSendQueue(std::string& msg)
{
	std::unique_lock<std::mutex> lock(sendQueueMutex);
	sendQueue.push(msg);
	lock.unlock();

	Send();
}

void Session::Send()
{
	while (!sendQueue.empty()) {
		std::unique_lock<std::mutex> lock(sendQueueMutex);
		std::string sendmsg = sendQueue.front();
		sendQueue.pop();
		lock.unlock();

		ZeroMemory(ctx->buffer, sizeof(ctx->buffer));
		ctx->oper = OPER::SEND;
		memcpy(ctx->buffer, sendmsg.c_str(), sendmsg.size());
		ctx->wsabuf.len = sendmsg.size();
		ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
		WSASend(ctx->clientSock, &ctx->wsabuf, 1, nullptr, ctx->flags, &ctx->overlapped, nullptr);
	}
}

void Session::Recv(size_t transferred)
{
	if (transferred == 0) {
		std::cout << "[Recv] transferred == 0 (early or disconnection)\n";
		typerecvbytes = 0;

		ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
		int ret = WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			std::cerr << "[Recv] WSARecv failed during re-arming: " << WSAGetLastError() << std::endl;
			SessionManager::GetInstance().closeSession(ctx->clientSock);
		}
		return;
	}
	if (ctx->type == TYPE::TYPE) {
		memcpy(type + typerecvbytes, ctx->buffer, transferred);
		Task tt;
		if (!tt.IsValidPACKTYPE(type[0])) {
			std::cout << "잘못된 패킷 아이디" << std::endl;
			typerecvbytes = 0;

			// !! 이거 패킷 이상하게 만들어서 D-Dos 오면 박살날거같은데.
		}
		else {
			std::cout << "Recv TYPE -> " << std::string(type, 1) << std::endl;
			typerecvbytes += transferred;
		}

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
			typerecvbytes = 0;

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
			std::cout << "Recv Len -> " << val << std::endl;
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
		std::cout << "Recv PayLoad -> " << payload << std::endl;

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


