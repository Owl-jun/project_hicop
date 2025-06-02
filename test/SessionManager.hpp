#pragma once
#include "Session.hpp"

class SessionManager
{
private:
	std::unordered_map<int , std::shared_ptr<Session>> sessions;
	SessionManager();
	~SessionManager();
public:
	static SessionManager& GetInstance();
	static int GenPk(); 
	void BroadCasting(std::string msg);
	void createSession(std::shared_ptr<Session> session);
	void closeSession(int pk);
	int FindBySocket(SOCKET sock);
	std::unordered_map<int, std::shared_ptr<Session>>& GetSessions();

};

