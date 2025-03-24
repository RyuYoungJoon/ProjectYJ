#include "pch.h"
#include "SessionManager.h"
#include "AsioSession.h"

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

void SessionManager::AddSession(AsioSessionPtr session)
{
	if (!session)
		return;

	std::lock_guard<std::mutex> lock(m_Mutex);
	int32 sessionUID = session->GetSessionUID();
	auto it = m_Sessions.find(sessionUID);
	if (it == m_Sessions.end())
		m_Sessions.insert(std::make_pair(sessionUID, session));
}

void SessionManager::RemoveSession(AsioSessionPtr session)
{
	if (!session)
		return;

	std::lock_guard<std::mutex> lock(m_Mutex);
	int32 sessionUID = session->GetSessionUID();
	m_Sessions.erase(sessionUID);
}

AsioSessionPtr SessionManager::GetSession(int32 sessionUID)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	auto it = m_Sessions.find(sessionUID);
	if (it != m_Sessions.end())
	{
		return it->second;
	}
	
	LOGE << "NotExists session! SessionUID : " << sessionUID;
	return nullptr;
}

