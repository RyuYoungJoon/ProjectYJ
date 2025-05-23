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
	void DummyClientProcess();
	
	bool GetIsStressTest() { return m_IsStressTest; }
	void SetIsStressTest(bool isStressTest) { m_IsStressTest = isStressTest; }

	const std::map<int32, AsioSessionPtr>& GetSessions() const {
		return m_Sessions;
	}

public:
	void StopClient();

private:
	RunningState m_RunningState = RunningState::None;
	AsioSessionPtr m_Session;
	shared_ptr<AsioService> m_Service;
	std::map<int32, AsioSessionPtr> m_Sessions;

	std::map<int32, int32> m_SessionSeqNum;

	std::mutex m_Mutex;
	bool run = false;
	int32 targetRandomCnt = 0;

	bool m_IsStressTest;
	std::condition_variable m_cv;
};

