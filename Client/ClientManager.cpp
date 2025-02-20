#include "pch.h"
#include "ClientManager.h"

ClientManager::ClientManager()
{
}

void ClientManager::Init(AsioSessionPtr session)
{
	m_Session = session;
	m_RunningState = RunningState::Connect;
	Start();
}

void ClientManager::Start()
{
	switch (m_RunningState)
	{
	case RunningState::Connect:
	{
		string message("sdfdsfewfewf", 128);
		Packet packet;
		std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
		std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
		packet.header.type = PacketType::YJ;
		packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
		std::memcpy(packet.payload, message.c_str(), message.size());
		packet.tail.value = 255;

		m_Session->Send(packet);
	}
		break;
	case RunningState::Disconnect:
		break;
	case RunningState::Send:
		break;
	case RunningState::Recv:
		break;
	default:
		break;
	}

}
