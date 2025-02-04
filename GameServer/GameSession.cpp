#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.Init();
}

GameSession::~GameSession()
{
	LOGD << "Delete GameSession";
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
	Packet* packet = reinterpret_cast<Packet*>(buffer);
	AsioSessionPtr gameSession = GetSession();


	m_PacketHandler.HandlePacket(gameSession, packet);

	return int32();
}

int GameSession::OnConnected()
{
	return 0;
}
