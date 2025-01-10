#include "pch.h"
#include "GameSession.h"

GameSession::GameSession()
{

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
	return int32();
}

void GameSession::OnConnected()
{
}
