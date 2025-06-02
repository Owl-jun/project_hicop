#include "pch.h"
#include "LogicWorker.hpp"
#include "CustomStructs.hpp"
#include "SessionManager.hpp"

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
				Task task = TaskQueue.front();
				TaskQueue.pop();
				lock.unlock();
				
				process(task);
			}
		}));
	}
}

void LogicWorker::PustTask(Task t)
{
	{
		std::lock_guard<std::mutex> lock(qMutex);
		TaskQueue.push(std::move(t));
	}
	cv.notify_one();
}

void LogicWorker::process(Task task)
{
	char rawType = task.type;
	auto session = task.session;
	std::string msg = task.packet;
	std::istringstream iss(msg);
	if (rawType == (char)1) {
		std::string chatmsg;
		std::string t;
		while (iss >> t) {
			chatmsg.append(t + " ");
		}
		std::cout << chatmsg << std::endl;
		SessionManager::GetInstance().BroadCasting(chatmsg);
		// session->pushSendQueue(chatmsg);
	}
	else if (rawType == (char)2) {

	}
	else {
		// 정의되지 않은 패킷 타입
		std::cout << "Invalid PACKTYPE: {}" << (int)rawType << std::endl;
	}

}
