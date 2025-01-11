#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.RegisterHandler(PacketType::defEchoString, [this](const Packet& packet)
		{
			// ���� �ʿ�
			//std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
			//std::string message(packet.payload, packet.header.size - sizeof(PacketHeader));
			//std::cout << "Server received: " << message << std::endl;

			// ���� ����
			//SendResponse("OK");
		}
	);
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend ȣ��" << endl;
}

void GameSession::OnDisconnected()
{
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	return int32();
}

void GameSession::OnConnected()
{
}
