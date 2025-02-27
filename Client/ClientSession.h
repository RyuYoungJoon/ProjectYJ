#pragma once
#include "AsioSession.h"

class ClientSession : public AsioSession
{
public:
	ClientSession(boost::asio::io_context* iocontext, tcp::socket socket)
		: AsioSession(iocontext, std::move(socket))
	{
	}

	~ClientSession()
	{
		LOGI << "Destroy ClientSession";
	}

	void OnSend(int32 len);
	int32 OnRecv(BYTE* buffer, int32 len);
	void OnConnected();
	void OnDisconnected();
	void SendPacket(const std::string& message);
};


