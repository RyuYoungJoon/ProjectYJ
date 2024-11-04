#include "pch.h"
#include "AsioServer.h"
#include "AsioNetwork.h"

void AsioNetwork::DoServerStart(uint16 port)
{
	m_Server = std::make_shared<AsioServer>(m_IoContext, port);
	std::cout << "Server Start" << std::endl;
}

void AsioNetwork::DoClientStart(const std::string& host, uint16 port)
{

}

void AsioNetwork::Run()
{
	try {
		std::cout << "Running AsioNetwork" << std::endl;
		m_IoContext.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}

void AsioNetwork::Stop()
{
	m_IoContext.stop();
	std::cout << "NetWork Release Start" << std::endl;
}
