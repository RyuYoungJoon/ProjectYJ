#pragma once

enum class RunningState : int
{
	None,
	Connect,
	Disconnect,
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

	void Init(AsioSessionPtr session);
	void Process();
	void MakeSendBuffer();


private:
	RunningState m_RunningState = RunningState::None;
	AsioSessionPtr m_Session;

	int32 targetRandomCnt = 0;
	int32 packetCount = 0;
};

