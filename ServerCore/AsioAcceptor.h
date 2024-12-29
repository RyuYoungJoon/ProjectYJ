#pragma once

class AsioServerService;

class AsioAcceptor
{
public:
	AsioAcceptor();
	~AsioAcceptor();

public:
	bool StartAccept(std::shared_ptr<AsioServerService> service);
	void CloseSocket();

private:

	void DoAccept(AsioSession* session, const boost::system::error_code& error);

	tcp::socket m_socket;
	std::shared_ptr<AsioServerService> m_service;
	tcp::acceptor m_Acceptor;
};

