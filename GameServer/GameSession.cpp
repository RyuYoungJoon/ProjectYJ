#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include "ServerAnalyzer.h"
#include "SessionManager.h"

GameSession::GameSession()
{
}

GameSession::GameSession(boost::asio::io_context* iocontext, tcp::socket* socket)
	: AsioSession(iocontext, socket)
{
}

GameSession::~GameSession()
{
	Reset();
}

void GameSession::OnSend(int32 len)
{
}

void GameSession::OnDisconnected()
{
	SessionManager::GetInstance().RemoveSession(GetSession());
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	//Packet* packet = reinterpret_cast<Packet*>(buffer);

	AsioSessionPtr gameSession = GetSession();
	ServerAnalyzer::GetInstance().IncrementRecvCnt();

	return len;
}

void GameSession::OnConnected()
{
	SessionManager::GetInstance().AddSession(GetSession());
}

void GameSession::Reset()
{
}
