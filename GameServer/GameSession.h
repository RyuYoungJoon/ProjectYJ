#pragma once
#include "AsioSession.h"
#include "PacketHandler.h"

class GameSession : public AsioSession
{
public:
	GameSession(boost::asio::io_context& iocontext, tcp::socket socket);
	~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;
	virtual int32 OnRecv(BYTE* buffer, int32 len) override;
	virtual void OnConnected() override;
};

