#include "pch.h"
#include "ClientSession.h"
#include "ClientManager.h"

extern ClientServicePtr clientService;

ClientManager::ClientManager()
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution<int> dist(10, 100);
	targetRandomCnt = dist(dre);
}

void ClientManager::Init(ClientServicePtr service)
{
	//m_Session = session;
	m_Service = service;
	m_RunningState = RunningState::Connect;

}

void ClientManager::Process()
{
	switch (m_RunningState)
	{
	case RunningState::Connect:
	case RunningState::Send:
	{
		//for (int i = 0; i < targetRandomCnt; ++i)
		//{
			string message("sdfdsfewfewf", 128);
			Packet packet;
			std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
			std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
			packet.header.type = PacketType::YJ;
			packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
			std::memcpy(packet.payload, message.c_str(), message.size());
			packet.tail.value = 255;

			m_Service->BroadCast(packet);
		//}

		m_RunningState = RunningState::Disconnect;
	}
		break;
	case RunningState::Disconnect:
	{
		//LOGI << "RunningState Disconnect [SessionUID : " << m_Session->GetSessionUID() << "]";

		//m_Session->Disconnect();
		//m_Session.reset();
		m_Service->CloseService();
		m_RunningState = RunningState::Reconnect;
	}
		break;
	case RunningState::Reconnect:
	{
		/*AsioSessionPtr newSession = m_Service->CreateSession(m_Service->iocontext, tcp::socket(m_Service->iocontext));
		m_Session = newSession;
		m_Session->Connect("127.0.0.1", "7777");*/

		m_Service->Start();

		m_RunningState = RunningState::Connect;
	}
		break;
	case RunningState::Recv:
		break;
	default:
		break;
	}

}
