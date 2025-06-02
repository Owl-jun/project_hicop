#pragma once
class Server
{
private:
	std::vector<std::thread> IOPool;
	void Thread_Work(HANDLE iocp);
public:
	Server();
	~Server();

	void Run();
};

