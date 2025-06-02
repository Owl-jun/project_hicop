#pragma once
#include "CustomStructs.hpp"


struct Task;

class Session : public std::enable_shared_from_this<Session>
{
public:
	void PushTask(Task msg);
	
public:
	Session(std::shared_ptr<IOContext> _ctx);
	~Session();

	void pushSendQueue(std::string& msg);
	void Send();
	void Recv(size_t transferred);
	void close();
	void ResetFSM();
	void SetPk(int _pk);
	SOCKET GetSocket();
	std::shared_ptr<IOContext> GetCtx() { return ctx; }
	int GetPK();
	void SetUID(std::string& id);
	std::string& GetUID();
private:
	int pk;
	std::string userId;
	std::shared_ptr<IOContext> ctx;
	std::queue<std::string> sendQueue;
	std::mutex sendQueueMutex;

private:
	char type;
	char len[4];
	std::string payload;
	int typerecvbytes = 0;
	int lenrecvbytes = 0;
	int plrecvbytes = 0;
	int packetLength = 0;
	bool isEnd = false;
};



struct Task {
	std::shared_ptr<Session> session;
	char type;
	std::string packet;
};