#pragma once
#include "CustomStructs.hpp"


struct Task;

class Session : public std::enable_shared_from_this<Session>
{
public:
	void PushTask(Task&& msg);
	
public:
	Session(std::shared_ptr<IOContext> _ctx);
	~Session();

	void Recv();
	void close();
	void SetPk(int _pk);
	SOCKET GetSocket();
	std::shared_ptr<IOContext> GetCtx() { return ctx; }
	int GetPK();
private:
	int pk;
	std::shared_ptr<IOContext> ctx;
	char type[1];
	char len[4];
	char payload[1024];
	int recvbytes;
};

struct Task {
	std::shared_ptr<Session> session;
	std::string rawData;
};