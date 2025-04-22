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

	// UI 관련 함수
	static void SetMainWin(HWND hwnd) { s_hMainWin = hwnd; }

	static HWND s_hMainWin;
private:
};


