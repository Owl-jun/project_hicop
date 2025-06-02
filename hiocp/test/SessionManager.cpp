#include "pch.h"
#include "SessionManager.hpp"

SessionManager::SessionManager() {

}

SessionManager::~SessionManager()
{
}

SessionManager& SessionManager::GetInstance()
{
	static SessionManager Instance;
	return Instance;
}

int SessionManager::GenPk()
{
	static int pk = 0;
	return pk++;
}

void SessionManager::BroadCasting(std::string msg)
{
	for (auto& [_, session] : sessions) {
		session->pushSendQueue(msg);
	}
}

void SessionManager::createSession(std::shared_ptr<Session> session)
{
	int pk = GenPk();
	session->SetPk(pk);
	sessions[pk] = session;
}

void SessionManager::closeSession(int pk)
{
	if (sessions.find(pk) != sessions.end()) {
		sessions[pk]->close();
		sessions.erase(pk);
	}
}

int SessionManager::FindBySocket(SOCKET sock) {
	for (auto& [_, session] : sessions) {
		if (session->GetSocket() == sock)
			return session->GetPK();
	}
	return -1;
}

std::unordered_map<int, std::shared_ptr<Session>>& SessionManager::GetSessions()
{
	return sessions;
}
