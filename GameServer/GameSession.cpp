#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include "ServerAnalyzer.h"
#include "SessionManager.h"

GameSession::GameSession()
{
	//m_PacketHandler.Init();
}

GameSession::GameSession(boost::asio::io_context* iocontext, tcp::socket* socket)
	: AsioSession(iocontext, socket)
{
	//m_PacketHandler.Init();
}

GameSession::~GameSession()
{
	Reset();
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend È£Ãâ" << endl;
}

void GameSession::OnDisconnected()
{
	//m_SessionPool.Push(shared_from_this());
	//m_PacketHandler.Reset(GetSessionUID());
	SessionManager::GetInstance().UnregisterSession(GetSession());
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet* packet = reinterpret_cast<Packet*>(buffer);

	//AsioSessionPtr gameSession = m_SessionPool.Pop();
	AsioSessionPtr gameSession = GetSession();
	ServerAnalyzer::GetInstance().IncrementRecvCnt();

	//m_PacketHandler.HandlePacket(gameSession, packet);
	
	return int32();
}

void GameSession::OnConnected()
{
	SessionManager::GetInstance().RegisterSession(GetSession());
}

void GameSession::Reset()
{
	LOGD << "Delete Session";
}
