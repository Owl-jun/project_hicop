
#include "pch.h"
#include "Session.hpp"
#include "SessionManager.hpp"
#include "LogicWorker.hpp"

bool IsValidPACKTYPE(char type) {
    return type == (char)1 ||
        type == (char)2 ||
        type == (char)4 ||
        type == (char)8;
}

Session::Session(std::shared_ptr<IOContext> _ctx)
{
    ctx = _ctx;
    ResetFSM();  // 초기화
}

Session::~Session()
{
}

void Session::ResetFSM()
{
    typerecvbytes = 0;
    lenrecvbytes = 0;
    plrecvbytes = 0;
    packetLength = 0;
    payload.clear();
    ctx->type = TYPE::TYPE;
    ctx->wsabuf.len = 1;
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
        ctx->wsabuf.buf = ctx->buffer;
        ctx->wsabuf.len = static_cast<ULONG>(sendmsg.size());
        ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
        WSASend(ctx->clientSock, &ctx->wsabuf, 1, nullptr, ctx->flags, &ctx->overlapped, nullptr);
    }
}

void Session::Recv(size_t transferred)
{
    if (transferred == 0) {
        std::cout << "[Recv] transferred == 0 (early or disconnection)";
        ResetFSM();
        ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
        WSARecv(ctx->clientSock, &ctx->wsabuf, 1, nullptr, &ctx->flags, &ctx->overlapped, nullptr);
        return;
    }

    if (ctx->type == TYPE::TYPE) {
        memcpy(type + typerecvbytes, ctx->buffer, transferred);
        typerecvbytes += transferred;
        
        if (typerecvbytes >= 1 && IsValidPACKTYPE(type[0])) {
            ctx->wsabuf.len = 4;
            ctx->type = TYPE::LENGHTH;
            typerecvbytes = 0;
        }
        else {
            ResetFSM();
        }
    }
    else if (ctx->type == TYPE::LENGHTH)
    {
        memcpy(len + lenrecvbytes, ctx->buffer, transferred);
        lenrecvbytes += transferred;

        if (lenrecvbytes >= 4) {
            int val = 0;
            memcpy(&val, len, 4);
            packetLength = ntohl(val);
            ctx->wsabuf.len = packetLength;
            ctx->type = TYPE::PAYLOAD;
            lenrecvbytes = 0;
        }
        else {
            ctx->wsabuf.len = 4 - lenrecvbytes;
        }
    }
    else if (ctx->type == TYPE::PAYLOAD)
    {
        payload.append(ctx->buffer, transferred);
        plrecvbytes += transferred;

        if (plrecvbytes >= packetLength)
        {
            Task t = { shared_from_this(), type[0], payload };
            LogicWorker::GetInstance().PustTask(t);
            ResetFSM();  // FSM 초기화
        }
        else {
            ctx->wsabuf.len = packetLength - plrecvbytes;
        }
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