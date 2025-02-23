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

	void Init(ClientServicePtr service);
	void Process();
	void MakeSendBuffer();


private:
	RunningState m_RunningState = RunningState::None;
	AsioSessionPtr m_Session;
	shared_ptr<AsioService> m_Service;

	int32 targetRandomCnt = 0;
	int32 packetCount = 0;
};

