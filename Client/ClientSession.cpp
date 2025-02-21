#include "pch.h"
#include "AsioService.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"

void ClientSession::OnSend(int32 len)
{
	//packetCount++;
	//
	//if (packetCount >= targetRandomCnt)
	//{
	//	//std::this_thread::sleep_for(1s);
	//	boost::asio::post(m_IoContext, [this]() {
	//		Disconnect();
	//		});
	//}
	LOGI << "OnSend È£Ãâ!";
}

int32 ClientSession::OnRecv(BYTE* buffer, int32 len)
{
	return int32();
}

void ClientSession::OnConnected()
{
	GetService()->AddSession(shared_from_this());
	AsioSessionPtr clientSession = GetSession();

	ClientManager::GetInstance().Init(clientSession);

	//Disconnect();
}

void ClientSession::OnDisconnected()
{
	LOGI << "Disconnected Server!";

	if (ServerAnalyzer::GetInstance().GetTotalSendCount() < 100000)
	{
		ServerAnalyzer::GetInstance().ResetSendCount();
		//packetCount = 0;
		//Connect(serverIP, serverPort);
	}

	std::this_thread::sleep_for(1s);
	LOGI << "TryConnected Server!";
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