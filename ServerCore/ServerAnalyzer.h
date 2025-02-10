#pragma once
class ServerAnalyzer
{
public:
	ServerAnalyzer()
		: m_SendCount(0), m_RecvCount(0) 
	{
	}

	~ServerAnalyzer() = default;

	static ServerAnalyzer& GetInstance()
	{
		static ServerAnalyzer instance;
		return instance;
	}

	void IncrementSendCnt()
	{
		m_SendCount.fetch_add(1);
	}

	void IncrementRecvCnt()
	{
		m_RecvCount.fetch_add(1);
	}

	int32 GetSendCount() const { return m_SendCount; }
	int32 GetRecvCount() const { return m_RecvCount; }

private:
	atomic<int32> m_SendCount;
	atomic<int32> m_RecvCount;
};

