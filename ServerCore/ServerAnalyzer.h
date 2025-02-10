#pragma once
class ServerAnalyzer
{
public:
	ServerAnalyzer()
		: m_SendCount(0), m_RecvCount(0), m_TotalRecvCount(0), m_TotalSendCount(0)
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
		m_TotalSendCount.fetch_add(1);
    }

	void IncrementRecvCnt()
	{
		m_RecvCount.fetch_add(1);
		m_TotalRecvCount.fetch_add(1);
	}

	void ResetSendCount()
	{
		m_SendCount.store(0);
	}

	void ResetRecvCount()
	{
		m_RecvCount.store(0);
	}

	int32 GetSendCount() const { return m_SendCount; }
	int32 GetRecvCount() const { return m_RecvCount; }

private:
	// pair <TotalCnt, Cnt>
	atomic<int32> m_SendCount;
	atomic<int32> m_RecvCount;
	atomic<int32> m_TotalSendCount;
	atomic<int32> m_TotalRecvCount;
};