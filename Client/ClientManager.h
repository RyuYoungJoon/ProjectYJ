#pragma once
#include "AsioService.h"

enum class RunningState : int
{
	None,
	Connect,
	Disconnect,
	Reconnect,
	Send,
	Recv
};

class ClientManager
{
public:
	ClientManager();

	static ClientManager& GetInstance()
	{
		static ClientManager instance;
		return instance;
	}

	void Init(int32 sessionUid, AsioSessionPtr service);
	void Process();
	void MakeSendBuffer();
	void ProcessStart();

private:
	RunningState m_RunningState = RunningState::None;
	AsioSessionPtr m_Session;
	shared_ptr<AsioService> m_Service;
	//std::set<AsioSessionPtr> m_Sessions;
	std::map<int32, AsioSessionPtr> m_Sessions;
	shared_ptr<boost::asio::steady_timer> m_Timer;

	std::mutex m_Mutex;
	bool run = false;
	int32 targetRandomCnt = 0;
	int32 packetCount = 0;
};

