#pragma once
#include "AsioSession.h"

using boost::asio::ip::tcp;

class AsioServer
{
public:
	AsioServer() = default;

	AsioServer(boost::asio::io_context& ioContext, uint16 port)
		: m_Acceptor(ioContext, tcp::endpoint(tcp::v4(), port)) 
	{
		DoAccept();
		std::cout << "Server Init" << std::endl;
	}

	~AsioServer();

public:

	// TODO : 상속받아서 Run Start HandlePacket 개발 해야함!
	//void Run(); 
	//void Start() override;
	//void HandlePacket(const std::string& packet) override;

private:
	void DoAccept();
	void DoRead(std::shared_ptr<tcp::socket> socket);
	void DoWrite(std::shared_ptr<tcp::socket> socket, const std::string& message);


private:
	boost::asio::io_context ioContext;
	tcp::acceptor m_Acceptor;
	std::unordered_set<std::shared_ptr<PacketSession>> m_sessions;
};

