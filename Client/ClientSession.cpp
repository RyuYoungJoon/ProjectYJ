#include "pch.h"
#include "AsioService.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"

ClientSession::ClientSession()
{
}

void ClientSession::OnSend(int32 len)
{
}

int32 ClientSession::OnRecv(BYTE* buffer, int32 len)
{
	return int32();
}

void ClientSession::OnConnected()
{
	AsioSessionPtr clientSession = GetSession();
	int32 sessionUID = GetSessionUID();
	clientSession->SetIsRunning(true);
	
	ClientManager::GetInstance().Init(sessionUID, clientSession);
	LOGI << "Conntect FINISH! SessiounUID : " << sessionUID << ", ThreadID : " << GetCurrentThreadId();
}

void ClientSession::OnDisconnected()
{
	LOGI << "Disconnected Server!";
	
	ServerAnalyzer::GetInstance().ResetSendCount();
	m_IsRunning = false;
}

void ClientSession::SendPacket(const std::string& message)
{
	Packet packet;
	std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
	std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
	packet.header.type = PacketType::YJ;
	packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
	std::memcpy(packet.payload, message.c_str(), message.size());
	packet.tail.value = 255;

	Send(packet);
}