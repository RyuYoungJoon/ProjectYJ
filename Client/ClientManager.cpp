#include "pch.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include "ClientManager.h"
#include "ServerAnalyzer.h"

extern ClientServicePtr clientService;
extern int32 maxSessionCnt;
extern int32 threadCnt;

ClientManager::ClientManager()
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution<int> dist(10, 100);
	targetRandomCnt = dist(dre);
}

void ClientManager::Init(int32 sessionUid, AsioSessionPtr session)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto iter = m_Sessions.find(sessionUid);
	if(iter == m_Sessions.end())
		m_Sessions.insert(std::make_pair(sessionUid, session));
	else
		LOGE << "Already be Session";
	
	m_Service = clientService;

	m_RunningState = RunningState::Connect;
	int32 sessionSize = m_Sessions.size();
	LOGD << "Session Size : " << sessionSize;

	// 스트레스 테스트 플래그
	if (m_IsStressTest == true)
	{
		if (sessionSize == threadCnt * maxSessionCnt)
		{
			run = true;
			m_cv.notify_all();
		}
	}
}

void ClientManager::DummyClientProcess()
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_cv.wait(lock, [this]() { return run; });
	lock.unlock();

	while (run)
	{
		switch (m_RunningState)
		{
		case RunningState::Connect:
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			for (auto& session : m_Sessions)
			{
				m_SessionSeqNum[session.first] = 0;
			}
			m_RunningState = RunningState::Send;
		}
		break;
		case RunningState::Send:
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			string messages("In multithreaded programming, it is crucial to use mutexes and condition variables");
			
			for (auto& session : m_Sessions)
			{
				int32 sessionId = session.first;
				PacketDummyClientMessage packet;
				memcpy(packet.payload.message, messages.c_str(), messages.length());

				for (int i = 0; i < 100; ++i)
				{
					session.second->Send(packet);
				}
			}
			
			m_RunningState = RunningState::Disconnect;
		}
		break;
		case RunningState::Disconnect:
		{
			// 클라이언트 Disconnect
			std::lock_guard<std::mutex> lock(m_Mutex);

			StopClient();
			
			m_RunningState = RunningState::Reconnect;
			
			run = false;
		}
		break;
		case RunningState::Reconnect:
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			m_Service->Start();

			m_RunningState = RunningState::Connect;
		}
		break;
		case RunningState::Recv:
			break;
		default:
			break;
		}

		LOGD << "Send Cnt : " << ServerAnalyzer::GetInstance().GetSendCount() << ", TotalSend Count : " << ServerAnalyzer::GetInstance().GetTotalSendCount();
		LOGD << "Session Size : " << m_Sessions.size();
		std::this_thread::sleep_for(1s);
	}
}

void ClientManager::StopClient()
{
	for (auto session : m_Sessions)
	{
		session.second->Disconnect();
		session.second->Reset();

		m_SessionSeqNum.erase(session.first);
	}
	m_Sessions.clear();
}

