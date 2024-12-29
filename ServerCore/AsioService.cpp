#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"

AsioService::AsioService(ServiceType type, boost::asio::io_context& iocontext, short port, SessionMaker sessionmaker, int32 maxSessionCount)
{
}

AsioService::~AsioService()
{
}

void AsioService::CloseService()
{
}

std::shared_ptr<class AsioSession> AsioService::CreateSession()
{
	std::shared_ptr<class AsioSession> session = m_SessionMaker();
	session->SetService(shared_from_this());

	return session;
}

void AsioService::AddSession(std::shared_ptr<class AsioSession> session)
{
	// TODO : Lock을 걸어줘야하나?

	m_SessionCount++;
	m_Sessions.insert(session);
}

void AsioService::ReleaseSession(std::shared_ptr<class AsioSession> session)
{
	m_Sessions.erase(session);
	m_SessionCount--;
}

AsioServerService::AsioServerService(boost::asio::io_context& iocontext, short port, SessionMaker sessionmaker, int32 maxSessionCount)
	:AsioService(ServiceType::Server, iocontext, port, sessionmaker, maxSessionCount), 
	m_IoContext(iocontext),
	m_Acceptor(iocontext, tcp::endpoint(tcp::v4(), port))
{
}

bool AsioServerService::Start()
{
	return false;
}

void AsioServerService::CloseService()
{
	// TODO
	int a = 0;

	AsioService::CloseService();
}

AsioClientService::AsioClientService(boost::asio::io_context& iocontext, short port, SessionMaker sessionmaker, int32 maxSessionCount)
	: AsioService(ServiceType::Client, iocontext, port, sessionmaker, maxSessionCount)
{
}
