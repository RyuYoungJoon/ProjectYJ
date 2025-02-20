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

	/*boost::asio::socket_base::reuse_address reuseAddrOpt(true);
	boost::asio::socket_base::linger lingerOpt(true, 0);
	boost::asio::ip::tcp::no_delay noDelayOpt(true);

	session->GetSocket().set_option(reuseAddrOpt);
	session->GetSocket().set_option(lingerOpt);
	session->GetSocket().set_option(noDelayOpt);
	session->GetSocket().set_option(boost::asio::socket_base::send_buffer_size(0));*/


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

void AsioService::Process()
{
	string message("sdsdfsdffqw", 128);
	Packet packet;
	std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
	std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
	packet.header.type = PacketType::YJ;
	packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
	std::memcpy(packet.payload, message.c_str(), message.size());
	packet.tail.value = 255;

	while (m_IsRunning)
	{
		LOGI << "Process On!";

		BroadCast(packet);

		std::this_thread::sleep_for(1s);
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
			m_IsRunning = true;
			LOGD << "Session : " << i << ", Socket Handle : " << session->GetSocket().native_handle();
		}
	}

	return true;
}