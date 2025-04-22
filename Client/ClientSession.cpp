#include "pch.h"
#include "AsioService.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"

HWND ClientSession::s_hMainWin = NULL;
HWND ClientSession::s_hLobbyWin = NULL;
HWND ClientSession::s_hChatWin = NULL;

ClientSession::ClientSession()
{
}

void ClientSession::OnSend(int32 len)
{
}

int32 ClientSession::OnRecv(BYTE* buffer, int32 len)
{
	return int32();
}

void ClientSession::OnConnected()
{
	SessionManager::GetInstance().AddSession(GetSession());
	AsioSessionPtr clientSession = GetSession();
	int32 sessionUID = GetSessionUID();
	clientSession->SetIsRunning(true);
	
	ClientManager::GetInstance().Init(sessionUID, clientSession);
	LOGI << "Conntect FINISH! SessiounUID : " << sessionUID << ", ThreadID : " << GetCurrentThreadId();

	// 메시지 포스트 하기.
	if (s_hMainWin != NULL)
	{
		PostMessage(s_hMainWin, WM_CLIENT_CONNECTED, 0, 0);
		//clientSession->Send("hi", PacketType::LoginReq);
	}
}

void ClientSession::OnDisconnected()
{
	LOGI << "Disconnected Server!";
	SessionManager::GetInstance().RemoveSession(GetSession());

	ServerAnalyzer::GetInstance().ResetSendCount();
	m_IsRunning = false;
}