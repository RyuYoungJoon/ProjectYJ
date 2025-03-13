#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "MemoryPoolManager.h"
#include "TaskQueue.h"
#include "NetworkHandler.h"
#include "ServerAnalyzer.h"
#include "PacketRouter.h"

atomic<int32> SessionUID = 0;

AsioSession::AsioSession()  
{  
	m_IoContext = nullptr;
	m_Socket = nullptr;  
	m_Resolver = nullptr;
}

AsioSession::AsioSession(boost::asio::io_context* iocontext, tcp::socket* socket)
	: m_IoContext(iocontext), m_PacketBuffer(65536)
{
	m_Resolver = nullptr;
	m_Socket = socket;
}

AsioSession::~AsioSession()
{
	Reset();
}

void AsioSession::ProcessRecv()
{
	OnConnected();

	DoRead();
}

void AsioSession::InitSession(boost::asio::io_context* ioContext, tcp::socket* socket)
{
	m_IoContext = ioContext;
	m_Socket = socket;
	m_PacketBuffer.Init(65536);
}

void AsioSession::Send(const Packet& message)
{
	ServerAnalyzer::GetInstance().IncrementSendCnt();
	// 메모리풀 매니저에서 메모리를 가져오자.
	size_t bufferSize = message.header.size;

	// 메모리풀에 버퍼 사이즈만큼 메모리를 가져와서 buffer 생성
	BYTE* buffer = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(bufferSize).Allocate());

	std::memcpy(buffer, &message.header, sizeof(PacketHeader));
	std::memcpy(buffer + sizeof(PacketHeader), message.payload, message.header.size - sizeof(PacketHeader));

	// TODO : 메모리 DeAllocate 구조잡기
	//auto self = shared_from_this();
	m_Socket->async_write_some(boost::asio::mutable_buffer(buffer, bufferSize),
		std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

	//LOGD << "Send Socket Handle : " << m_Socket.lowest_layer().native_handle();
	//MemoryPoolManager::GetMemoryPool(bufferSize).Deallocate(buffer);
}

bool AsioSession::Connect(const string& host, const string& port)
{
	m_Resolver = new tcp::resolver(*m_IoContext);
	m_Socket = new tcp::socket(*m_IoContext);
	// 새로운 소켓 생성
	tcp::resolver::query targetQuery(host, port);
	auto targetEndpoint = m_Resolver->resolve(targetQuery);
	
	auto self = shared_from_this();
	boost::asio::async_connect(*m_Socket, targetEndpoint,
		[this, self](boost::system::error_code ec, tcp::endpoint endpoint)
		{
			if (!ec)
			{
				LOGI << "Successfully connected to " << endpoint;
				m_SessionUID = SessionUID.fetch_add(1);
				ProcessRecv();
			}
			else
			{
				LOGE << "Connection Failed : " <<ec.value()<< ", " << ec.message();
			}
		});

	return true;
}

void AsioSession::Disconnect()
{
	boost::system::error_code ec;

	LOGI << "SessionUID : " << GetSessionUID() << ", Disconnecting";

	OnDisconnected();
	CloseSession(__FUNCTION__);

}

void AsioSession::SetService(std::shared_ptr<AsioService> service)
{
	m_Service = service;
}

void AsioSession::DoRead()
{
	ServerAnalyzer::GetInstance().ResetRecvCount();

	m_Socket->async_read_some(boost::asio::buffer(m_PacketBuffer.WritePos(), m_PacketBuffer.FreeSize()),
		std::bind(&AsioSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::HandleRead(boost::system::error_code ec, int32 length)
{
	if (!ec)
	{
		if (m_PacketBuffer.OnWrite(length) == false)
		{
			LOGE << "OnWrite OverFlow";
			CloseSession(__FUNCTION__);
			return;
		}

		int32 dataSize = m_PacketBuffer.DataSize();
		std::vector<BYTE> dataCopy(dataSize);
		memcpy(dataCopy.data(), m_PacketBuffer.ReadPos(), dataSize);

		NetworkHandler::GetInstance().RecvData(shared_from_this(), dataCopy.data(), dataSize);
		// 다음 비동기 읽기 시작
		m_PacketBuffer.OnRead(length);
		m_PacketBuffer.Clear();

		DoRead();
	}
	else if (ec == boost::asio::error::eof)
	{
		LOGE << "Connection closed by peer";
		CloseSession(__FUNCTION__);
		return;
	}
	else if (ec == boost::asio::error::operation_aborted)
	{
		LOGI << "Operation Aboreted!";
		return;
	}
	else
	{
		if (ec.value() == boost::asio::error::connection_reset)
			LOGE << "CloseSession";
		else
			LOGE << "Read error : " << ec.message() << " (code : " << ec.value() << ")";

		return;
	}
}

void AsioSession::HandleWrite(boost::system::error_code ec, int32 length)
{
	if (ec)
	{
		LOGE << "Session Close : " << ec.value() << ", Message : " << ec.message();

		CloseSession(__FUNCTION__);
	}   
	else
	{
		OnSend(length);
	}
}

int32 AsioSession::ProcessPacket(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		int32 dataSize = len - processLen;
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (dataSize < header.size)
			break;

		OnRecv(&buffer[processLen], header.size);
		PacketRouter::GetInstance().Dispatch(shared_from_this(), &buffer[)

		processLen += header.size;
	}

	return processLen;

}

void AsioSession::CloseSession(const char* pCallFunc)
{
	LOGD << "CloseSession Called! " << pCallFunc << ", socket handle : " << m_Socket->native_handle();
	std::lock_guard<std::mutex> lock(m_Mutex);

	boost::system::error_code ec;
	m_Socket->cancel(ec);
	if (ec) {
		LOGE << "Cancel error: " << ec.value() << ", " << ec.message();
	}

	if (m_Socket->is_open())
	{
		m_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		if (ec)
		{
			LOGE << "ShutDown Error : " << ec.value() << ", " << ec.message();
		}
		m_Socket->close(ec);
		if (ec)
		{
			LOGE << "Close Error : " << ec.value() << ", " << ec.message();
		}
	}

	if (auto service = m_Service.lock())
	{
		service->ReleaseSession(shared_from_this());
	}

	//Reset();
}

void AsioSession::Reset()
{
	LOGD << "Destroy AsioSession";

	delete m_Socket;
	m_Socket = nullptr;

	delete m_Resolver;
	m_Resolver = nullptr;
}