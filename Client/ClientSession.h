#pragma once
#include "AsioSession.h"

#define WM_CLIENT_CONNECTED (WM_APP + 1)
#define WM_CLIENT_DISCONNECT (WM_APP + 2)
#define WM_CLIENT_RECV (WM_APP + 3)

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
	void SendPacket(const std::string& message, const PacketType packetType);

	// UI 관련 함수
	static void SetMainWin(HWND hwnd) { s_hMainWin = hwnd; }

private:
	static HWND s_hMainWin;
};


