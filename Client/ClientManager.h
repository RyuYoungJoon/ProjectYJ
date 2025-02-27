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

private:
	void StopClient();

private:
	RunningState m_RunningState = RunningState::None;
	AsioSessionPtr m_Session;
	shared_ptr<AsioService> m_Service;
	std::map<int32, AsioSessionPtr> m_Sessions;

	std::mutex m_Mutex;
	bool run = false;
	int32 targetRandomCnt = 0;

	std::condition_variable m_cv;
};

