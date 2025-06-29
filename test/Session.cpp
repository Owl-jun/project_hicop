
#include "pch.h"
#include "Session.hpp"
#include "SessionManager.hpp"
#include "LogicWorker.hpp"

bool IsValidPACKTYPE(char type) {
    return type == (char)1 ||
        type == (char)2 ||
        type == (char)4 ||
        type == (char)5 ||
        type == (char)8;
}

Session::Session(std::shared_ptr<IOContext> _ctx)
{
    ctx = _ctx;
    ResetFSM();  // �ʱ�ȭ
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
    ctx->wsabuf.buf = ctx->buffer;
    ctx->wsabuf.len = 1;
}

void Session::SetUID(std::string& id)
{
    userId = id;
}

std::string& Session::GetUID()
{
    return userId;
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
        char buf[5];
        buf[0] = 0x05;
        int msglen = sendmsg.size();
        msglen = htonl(msglen);
        memcpy(buf + 1, &msglen, 4);
        memcpy(ctx->buffer, buf, 5);
        memcpy(ctx->buffer + 5, sendmsg.c_str(), sendmsg.size());
        ctx->wsabuf.buf = ctx->buffer;
        ctx->wsabuf.len = static_cast<ULONG>(sendmsg.size()+5);
        ZeroMemory(&ctx->overlapped, sizeof(ctx->overlapped));
        WSASend(ctx->clientSock, &ctx->wsabuf, 1, nullptr, ctx->flags, &ctx->overlapped, nullptr);
    }
}

void Session::Recv(size_t transferred)
{
    if (transferred == 0) {
        std::cout << "[Recv] transferred == 0 (early or disconnection)" << std::endl;

        auto pk = SessionManager::GetInstance().FindBySocket(ctx->clientSock);
        SessionManager::GetInstance().closeSession(pk); // ���⼭ closesocket, ���� ���� �� ����

        return; // �ٽ� Recv ȣ�� X
    }

    if (ctx->type == TYPE::TYPE) {
        type = ctx->buffer[0];
        typerecvbytes += transferred;
        
        if (typerecvbytes >= 1 && IsValidPACKTYPE(type)) {
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
            Task t = { shared_from_this(), type, payload };
            LogicWorker::GetInstance().PustTask(t);
            std::cout << payload << std::endl;
            ResetFSM();  // FSM �ʱ�ȭ
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