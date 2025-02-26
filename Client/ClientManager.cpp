#include "pch.h"
#include "ClientSession.h"
#include "ClientManager.h"

//extern ClientServicePtr clientService;

ClientManager::ClientManager()
	//: m_Timer(std::make_shared<boost::asio::steady_timer>(clientService->iocontext))
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution<int> dist(10, 100);
	targetRandomCnt = dist(dre);
}

void ClientManager::Init(int32 sessionUid, AsioSessionPtr session)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	if(m_Sessions.find(sessionUid) == m_Sessions.end())
		m_Sessions.insert(std::make_pair(sessionUid, session));
	else
		LOGE << "Already be Session";
	
	//m_Service = clientService;
	//session.reset();
	m_RunningState = RunningState::Connect;
	int32 sessionSize = m_Sessions.size();
	LOGD << "Session Size : " << sessionSize;
	//ProcessStart();
}

void ClientManager::Process()
{
	switch (m_RunningState)
	{
	case RunningState::Connect:
	case RunningState::Send:
	{
		for (int i = 0; i < targetRandomCnt; ++i)
		{
			string message("sdfdsfewfewf", 128);
			Packet packet;
			std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
			std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
			packet.header.type = PacketType::YJ;
			packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
			std::memcpy(packet.payload, message.c_str(), message.size());
			packet.tail.value = 255;

			//m_Session->Send(packet);
			//clientService->BroadCast(packet);
		}

		m_RunningState = RunningState::Disconnect;
	}
	break;
	case RunningState::Disconnect:
	{
		// 클라이언트 Disconnect
		StopClient();

		m_RunningState = RunningState::Reconnect;

		// Session 재 생성 후 connect.
		m_Session = m_Service->CreateSession(m_Service->iocontext, tcp::socket(m_Service->iocontext));
		m_Session->Connect("127.0.0.1", "7777");

		m_RunningState = RunningState::Connect;
	}
	break;
	case RunningState::Reconnect:
	{
		m_Session = m_Service->CreateSession(m_Service->iocontext, tcp::socket(m_Service->iocontext));
		m_Session->Connect("127.0.0.1", "7777");

		m_RunningState = RunningState::Connect;
	}
	break;
	case RunningState::Recv:
		break;
	default:
		break;
	}
}

void ClientManager::ProcessStart()
{
	if (m_Session == nullptr || m_Service == nullptr) 
		return;

	m_Timer->expires_after(std::chrono::milliseconds(500ms));
	m_Timer->async_wait([this](const boost::system::error_code& ec) {
		if (!ec && m_Session->GetIsRunning()) {
			Process();
			ProcessStart(); // 다시 실행하여 주기적으로 Process 실행
		}
		});
}

void ClientManager::StopClient()
{
	// timer cancle
	if (m_Timer)
	{
		boost::system::error_code ec;
		m_Timer->cancel(ec);
		if (ec)
		{
			LOGE << "m_Timer 에러 : " << ec.value() << ", " << ec.message();
		}
	}

	for (auto session : m_Sessions)
	{
		session.second->Disconnect();
		session.second.reset();
	}

	m_Sessions.clear();
}
