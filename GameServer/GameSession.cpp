#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include "ServerAnalyzer.h"

GameSession::GameSession()
{
}

GameSession::GameSession(boost::asio::io_context* iocontext, tcp::socket socket)
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
	cout << "OnSend È£Ãâ" << endl;
}

void GameSession::OnDisconnected()
{
	m_SessionPool.Push(shared_from_this());
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet* packet = reinterpret_cast<Packet*>(buffer);

	AsioSessionPtr gameSession = m_SessionPool.Pop();
	gameSession = GetSession();

	ServerAnalyzer::GetInstance().IncrementRecvCnt();

	m_PacketHandler.HandlePacket(gameSession, packet);

	return int32();
}

void GameSession::OnConnected()
{
}

void GameSession::Reset()
{
	GameSession::~GameSession();
}
