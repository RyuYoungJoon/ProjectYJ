#pragma once

using boost::asio::ip::tcp;

class AsioServer
{
public:
	AsioServer(boost::asio::io_context& ioContext, uint16 port)
		: m_Acceptor(ioContext, tcp::endpoint(tcp::v4(), port)) {
		DoAccept();
		std::cout << "Server Init" << std::endl;
	}

private:
	void DoAccept();
	void DoRead(std::shared_ptr<tcp::socket> socket);
	void DoWrite(std::shared_ptr<tcp::socket> socket, const std::string& message);

private:
	tcp::acceptor m_Acceptor;
};

