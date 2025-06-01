#pragma once
#include "CustomStructs.hpp"
#include "Session.hpp"



class LogicWorker
{
private:
	LogicWorker();
	~LogicWorker();
	std::queue<Task> TaskQueue;
	std::vector<std::thread> Workers;
	std::mutex qMutex;
	std::condition_variable cv;

public:
	static LogicWorker& GetInstance() {
		static LogicWorker Instance;
		return Instance;
	}
	void Run();
	void PustTask(Task t);
	void process(std::string msg);
};

