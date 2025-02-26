#include "pch.h"
#include "AsioSession.h"
#include "AsioAcceptor.h"
#include "AsioService.h"

int totalCnt = 0;
int RealTryCnt = 0;

AsioService::AsioService(ServiceType type, boost::asio::io_context& iocontext, string& host, string& port, SessionMaker SessionMaker, int32 maxSessionCount)
	:m_type(type), iocontext(iocontext), m_Host(host), m_Port(port), m_SessionMaker(SessionMaker), m_MaxSessionCount(maxSessionCount)
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
	AddSession(session);
	session->SetSessionUID(m_SessionCount);

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

void AsioService::BroadCast(const Packet& packet)
{
	for (auto session : m_Sessions)
	{
		session->Send(packet);
	}
}

AsioServerService::AsioServerService(boost::asio::io_context& iocontext, string& host, string& port, SessionMaker SessionMaker, int32 maxSessionCount)
	:AsioService(ServiceType::Server, iocontext, host, port, SessionMaker, maxSessionCount),
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

AsioClientService::AsioClientService(boost::asio::io_context& iocontext, string& host, string& port, SessionMaker SessionMaker, int32 maxSessionCount)
	: AsioService(ServiceType::Client, iocontext, host, port, SessionMaker, maxSessionCount),
	m_Socket(iocontext)
{
}

bool AsioClientService::Start()
{
	if (!CanStart())
		return false;

	const int32 maxSessionCount = GetMaxSessionCount();
	for (int i = 0; i < maxSessionCount; ++i)
	{
		AsioSessionPtr session = CreateSession(iocontext, tcp::socket(iocontext));
		if (session->Connect(m_Host, m_Port) == false)
		{
			LOGE << "Connect Fail!";
			return false;
		}
		else
		{
			session->SetSessionUID(i);
			LOGD << "Session : " << i << ", Socket Handle : " << session->GetSocket().native_handle();
		}
	}

	return true;
}