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
	LOGI << "OnSend 호출!";
}

int32 ClientSession::OnRecv(BYTE* buffer, int32 len)
{
	return int32();
}

void ClientSession::OnConnected()
{
	GetService()->AddSession(shared_from_this());
	AsioSessionPtr clientSession = GetSession();
	clientSession->SetIsRunning(true);

	//ClientManager::GetInstance().Init(clientSession);

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

	/*AsioSessionPtr clientSession = GetSession();
	boost::system::error_code ec;

	LOGI << "Disconnect Socket Handle Value : " << clientSession->GetSocket().lowest_layer().native_handle();
	clientSession->GetSocket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	if (ec)
	{
		LOGE << "ShutDown 에러 : " << ec.value() << ", " << ec.message() << ", " << ec.category().name();
	}

	clientSession->GetSocket().close(ec);
	if (ec)
	{
		LOGE << "Close 에러 : " << ec.value() << ", " << ec.message() << ", " << ec.category().name();
	}*/

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