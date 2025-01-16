#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include "Logger.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.Init();
}

GameSession::~GameSession()
{
	cout << Logger::DLog("[SERVER INFO] Delete GameSession") << endl;
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend È£Ãâ" << endl;
}

void GameSession::OnDisconnected()
{
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet* packet = reinterpret_cast<Packet*>(buffer);
	m_PacketHandler.HandlePacket(packet);


	return int32();
}

void GameSession::OnConnected()
{
}
