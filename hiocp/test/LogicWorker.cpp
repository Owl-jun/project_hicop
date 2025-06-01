#include "pch.h"
#include "LogicWorker.hpp"
#include "CustomStructs.hpp"

LogicWorker::LogicWorker()
{
}

LogicWorker::~LogicWorker()
{
	for (int i = 0; i < 4; ++i) {
		Workers[i].join();
	}
}

void LogicWorker::Run() {
	for (int i = 0; i < 4; ++i)
	{
		Workers.push_back(std::thread([&]() {
			while (true) {
				std::unique_lock<std::mutex> lock(qMutex);
				cv.wait(lock, [&] {return !TaskQueue.empty(); });


				std::cout << "로직워커진입" << std::endl;
				Task task = std::move(TaskQueue.front());
				TaskQueue.pop();
				lock.unlock();

				auto ctx = task.session->GetCtx();

			}
		}));
	}
}

void LogicWorker::PustTask(Task&& t)
{
	{
		std::lock_guard<std::mutex> lock(qMutex);
		TaskQueue.push(std::move(t));
	}
	cv.notify_one();
}

void LogicWorker::process(std::string msg)
{
	std::istringstream iss(msg);
	std::string payload;
	
}
