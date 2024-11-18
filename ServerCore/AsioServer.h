#pragma once
#include "AsioSession.h"
#include "AsioIoContext.h"

using boost::asio::ip::tcp;

class AsioServer
{
public:
	AsioServer(boost::asio::io_context& ioContext, uint16 port)
		: m_Acceptor(ioContext, tcp::endpoint(tcp::v4(), port))
	{
		DoAccept();
		std::cout << "Server Init" << std::endl;
	}

	~AsioServer();

public:

	// TODO : ��ӹ޾Ƽ� Run Start HandlePacket ���� �ؾ���!
	//void Run(); 
	//void Start() override;
	//void HandlePacket(const std::string& packet) override;

private:
	void DoAccept();


private:
	AsioIoContext* m_ioContext;
	tcp::acceptor m_Acceptor;
};

