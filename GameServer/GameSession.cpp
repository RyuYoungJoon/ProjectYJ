#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.RegisterHandler(PacketType::defEchoString , std::bind(&PacketHandler::HandledefEchoString, &m_PacketHandler, std::placeholders::_1));
	m_PacketHandler.RegisterHandler(PacketType::JH, std::bind(&PacketHandler::HandleJH, &m_PacketHandler, std::placeholders::_1));
	m_PacketHandler.RegisterHandler(PacketType::YJ, std::bind(&PacketHandler::HandleYJ, &m_PacketHandler, std::placeholders::_1));
	m_PacketHandler.RegisterHandler(PacketType::ES, std::bind(&PacketHandler::HandleES, &m_PacketHandler, std::placeholders::_1));

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
	Packet* packet = reinterpret_cast<Packet*>(buffer);
	m_PacketHandler.HandlePacket(packet);


	// 버퍼 초기화
	m_PacketBuffer.DiscardReadData();

	return int32();
}

void GameSession::OnConnected()
{
}
