#pragma once
#include "AsioSession.h"

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
	void SendPacket(const std::string& message);
};


