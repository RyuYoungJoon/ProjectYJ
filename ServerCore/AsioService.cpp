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
    std::lock_guard<std::mutex> lock(m_Mutex);

    // 모든 세션 종료
    for (auto& session : m_Sessions)
    {
        if (session)
        {
            session->CloseSession();
        }
    }

    // 세션 컨테이너 비우기
    m_Sessions.clear();
    m_SessionCount = 0;
    LOGI << "Session Clear";
}

AsioSessionPtr AsioService::CreateSession(boost::asio::io_context& iocontext, tcp::socket socket)
{
	AsioSessionPtr session = m_SessionMaker(iocontext, std::move(socket));
	session->SetService(shared_from_this());

	return session;
}

void AsioService::AddSession(AsioSessionPtr session)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

	m_Sessions.insert(session);
    m_SessionCount++;
}

void AsioService::ReleaseSession(AsioSessionPtr session)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

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
    if (m_Acceptor)
    {
        m_Acceptor->Stop();
    }

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
                            LOGI << "Successfully connected to " << endpoint;
                           
                            auto session = CreateSession(iocontext, std::move(m_Socket));
                            session->Start();

                            AddSession(session);
                        }
                        else
                        {
                            LOGE << "Connection Failed : " << ec.message();
                        }
                    });
            }
            else
            {
                LOGE << "Resolve Failed : " << ec.message();
            }
        });
}
