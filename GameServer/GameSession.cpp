#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include "ServerAnalyzer.h"

GameSession::GameSession()
{
	m_PacketHandler.Init();
}

GameSession::GameSession(boost::asio::io_context* iocontext, tcp::socket* socket)
	: AsioSession(iocontext, socket)
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
	//m_SessionPool.Push(shared_from_this());
	m_PacketHandler.Reset(GetSessionUID());
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet* packet = reinterpret_cast<Packet*>(buffer);

	//AsioSessionPtr gameSession = m_SessionPool.Pop();
	AsioSessionPtr gameSession = GetSession();
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
