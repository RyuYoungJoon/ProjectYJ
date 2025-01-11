#include "pch.h"
#include "AsioSession.h"
#include "AsioAcceptor.h"
#include "AsioService.h"

AsioService::AsioService(ServiceType type, boost::asio::io_context& iocontext, short port, SessionMaker SessionMaker, int32 maxSessionCount)
	:m_type(type), iocontext(iocontext), m_Port(port), m_SessionMaker(SessionMaker)
{
}

AsioService::~AsioService()
{
}

void AsioService::CloseService()
{
}

std::shared_ptr<class AsioSession> AsioService::CreateSession(boost::asio::io_context& iocontext, tcp::socket socket)
{
	std::shared_ptr<class AsioSession> session = m_SessionMaker(iocontext, std::move(socket));
	session->SetService(shared_from_this());

	return session;
}

void AsioService::AddSession(std::shared_ptr<class AsioSession> session)
{
	// TODO : Lock을 걸어줘야하나?;
	m_SessionCount++;
	m_Sessions.insert(session);
}

void AsioService::ReleaseSession(std::shared_ptr<class AsioSession> session)
{
	m_Sessions.erase(session);
	m_SessionCount--;
}

AsioServerService::AsioServerService(boost::asio::io_context& iocontext, short port, SessionMaker SessionMaker, int32 maxSessionCount)
	:AsioService(ServiceType::Server, iocontext, port, SessionMaker, maxSessionCount), 
	m_IoContext(iocontext)
{
}

AsioServerService::~AsioServerService()
{
}

bool AsioServerService::Start()
{
    if (!CanStart())
        return false;

    m_Acceptor = std::make_shared<AsioAcceptor>(m_IoContext, m_Port, shared_from_this());

	m_Acceptor->Start();

	return true;
}

void AsioServerService::CloseService()
{
	// TODO
	int a = 0;

	AsioService::CloseService();
}

AsioClientService::AsioClientService(boost::asio::io_context& iocontext, const std::string& host, short port, SessionMaker SessionMaker, int32 maxSessionCount)
	: AsioService(ServiceType::Client, iocontext, port, SessionMaker, maxSessionCount),
	m_Resolver(iocontext),
	m_Host(host),
	m_Port(port),
	m_Socket(iocontext)
{
}

bool AsioClientService::Start()
{
    if (!CanStart())
        return false;

    DoConnect();
    return true;
}

void AsioClientService::DoConnect()
{
    auto self(shared_from_this());

    m_Resolver.async_resolve(m_Host, std::to_string(m_Port),
        [this, self](const boost::system::error_code& ec, tcp::resolver::results_type results)
        {
            if (!ec)
            {
                boost::asio::async_connect(m_Socket, results,
                    [this, self](boost::system::error_code ec, tcp::endpoint endpoint)
                    {
                        if (!ec)
                        {
                            std::cout << "[INFO] Successfully connected to " << endpoint << std::endl;
                           
                            auto session = CreateSession(iocontext, std::move(m_Socket));
                            session->Start();
                        }
                        else
                        {
                            std::cerr << "[ERROR] Connection failed: " << ec.message() << std::endl;
                        }
                    });
            }
            else
            {
                std::cerr << "[ERROR] Resolve failed: " << ec.message() << std::endl;
            }
        });
}
