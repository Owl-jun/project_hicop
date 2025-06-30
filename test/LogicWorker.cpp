#include "pch.h"
#include "LogicWorker.hpp"
#include "CustomStructs.hpp"
#include "SessionManager.hpp"
#include "timetest.hpp"

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
				//timetest::GetInstance().go();
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
	///// TEST
	if (rawType == (char)1) {
		std::string chatmsg = session->GetUID() + " : ";
		std::string t;
		while (iss >> t) {
			chatmsg.append(t + " ");
		}
		if (chatmsg[0] == ' ') { chatmsg.erase(chatmsg.begin()); }
		if (chatmsg[chatmsg.size() - 1] == ' ') { chatmsg.erase(chatmsg.end() - 1); }
		std::cout << chatmsg << std::endl;
		SessionManager::GetInstance().BroadCasting(chatmsg);
		// session->pushSendQueue(chatmsg);
	}
	else if (rawType == (char)2) {
		std::string uid;
		iss >> uid;
		session->SetUID(uid);
	}
	/////
	else if (rawType == (char)5) {
		if (session->GetUID() == "") {
			std::string uid;
			iss >> uid;
			session->SetUID(uid);
		}
		else
		{
			std::string dummy;
			iss >> dummy;
		}
		std::string chatmsg = session->GetUID() + " : ";
		std::string t;
		while (iss >> t) {
			chatmsg.append(t);
		}
		if (chatmsg[0] == ' ') { chatmsg.erase(chatmsg.begin()); }
		if (chatmsg[chatmsg.size() - 1] == ' ') { chatmsg.erase(chatmsg.end() - 1); }
		std::cout << chatmsg << std::endl;
		SessionManager::GetInstance().BroadCasting(chatmsg);
		// session->pushSendQueue(chatmsg);
	}
	else {
		// 정의되지 않은 패킷 타입
		std::cout << "Invalid PACKTYPE: {}" << (int)rawType << std::endl;
	}

}
