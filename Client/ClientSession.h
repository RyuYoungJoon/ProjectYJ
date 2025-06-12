#pragma once
#include "AsioSession.h"

struct ChatMessageData
{
	string sender;
	string message;
};

class ClientSession : public AsioSession
{
public:
	ClientSession();
	ClientSession(boost::asio::io_context* iocontext, tcp::socket* socket)
		: AsioSession(iocontext, socket)
	{
	}

	~ClientSession()
	{
		LOGI << "Destroy ClientSession";
	}

	virtual void OnSend(int32 len) override;
	virtual int32 OnRecv(BYTE* buffer, int32 len) override;
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;

	// UI ���� �Լ�
	static void SetMainWin(HWND hwnd) { s_hMainWin = hwnd; }
	static void SetLobbyWin(HWND hwnd) { s_hLobbyWin = hwnd; }
	static void SetChatWin(HWND hwnd) { s_hChatWin = hwnd; }

	static HWND s_hMainWin;
	static HWND s_hLobbyWin;
	static HWND s_hChatWin;
private:
};


