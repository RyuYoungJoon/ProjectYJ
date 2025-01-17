#include "pch.h"
#include "GameSession.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.RegisterHandler(PacketType::defEchoString, [this](const Packet& packet)
		{
			// 정리 필요
			//std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
			//std::string message(packet.payload, packet.header.size - sizeof(PacketHeader));
			//std::cout << "Server received: " << message << std::endl;

			// 응답 전송
			//SendResponse("OK");
			std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
			std::cout << "Server Received : " << message << std::endl;
		}
	);
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend 호출" << endl;
}

void GameSession::OnDisconnected()
{
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet packet = reinterpret_cast<Packet&>(buffer);
	m_PacketHandler.HandlePacket(packet);
}

void GameSession::OnConnected()
{
}
