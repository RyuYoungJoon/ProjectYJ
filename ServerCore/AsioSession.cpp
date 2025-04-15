#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "NetworkHandler.h"
#include "ServerAnalyzer.h"
#include "PacketRouter.h"
#include "ObjectPool.h"

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

void AsioSession::ProcessDisconnect(const char* pCallback)
{
	// 이미 Disconnect임.
	if (m_NetState == NetState::Disconnect)
		return;

	LOGI << "ProcessDisconnect, SessionUID : " << GetSessionUID();

	//OnDisconnected();

	SetNetState(NetState::Disconnect);

	TaskQueue::GetInstance().PushTask(GetSessionUID(), GetNetState());
}

void AsioSession::InitSession(boost::asio::io_context* ioContext, tcp::socket* socket)
{
	m_IoContext = ioContext;
	m_Socket = socket;
	m_PacketBuffer.Init(65536);
}

void AsioSession::Send(const std::string& message, const PacketType packetType)
{
	ServerAnalyzer::GetInstance().IncrementSendCnt();

	// PacketPool에서 패킷 가져오기.
	Packet* sendPacket = PacketPool::GetInstance().Pop();

	std::memset(sendPacket->header.checkSum, 0x12, sizeof(sendPacket->header.checkSum));
	std::memset(sendPacket->header.checkSum + 1, 0x34, sizeof(sendPacket->header.checkSum) - 1);
	sendPacket->header.type = packetType;
	sendPacket->header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(sendPacket->payload) + sizeof(PacketTail));
	std::memcpy(sendPacket->payload, message.c_str(), message.size());
	sendPacket->tail.value = 255;

	m_Socket->async_write_some(boost::asio::mutable_buffer(reinterpret_cast<BYTE*>(sendPacket), sendPacket->header.size),
		std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2, sendPacket));
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
			ProcessDisconnect(__FUNCTION__);
			return;
		}

		int32 dataSize = m_PacketBuffer.DataSize();
		std::vector<BYTE> dataCopy(dataSize);
		memcpy(dataCopy.data(), m_PacketBuffer.ReadPos(), dataSize);

		NetworkHandler::GetInstance().RecvData(shared_from_this(), dataCopy.data(), dataSize);
		// 다음 비동기 읽기 시작
		dataCopy.clear();
		m_PacketBuffer.OnRead(length);
		m_PacketBuffer.Clear();

		DoRead();
	}
	else if (ec == boost::asio::error::eof)
	{
		LOGE << "Connection closed by peer";
		ProcessDisconnect(__FUNCTION__);
		return;
	}
	else if (ec == boost::asio::error::operation_aborted || ec == boost::asio::error::connection_reset)
	{
		return;
	}
	else
	{	
		LOGE << "Read error : " << ec.message() << " (code : " << ec.value() << ")";
		
		ProcessDisconnect(__FUNCTION__);
		return;
	}
}

void AsioSession::HandleWrite(boost::system::error_code ec, int32 length, Packet* packet)
{
	if (ec)
	{
		LOGE << "Session Close : " << ec.value() << ", Message : " << ec.message();

		ProcessDisconnect(__FUNCTION__);
	}   
	else
	{
		OnSend(length);
	}

	PacketPool::GetInstance().Push(packet);
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

		PacketRouter::GetInstance().Dispatch(shared_from_this(), &buffer[processLen]);
		processLen += header.size;
	}

	return processLen;

}

void AsioSession::CloseSession(const char* pCallFunc)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Socket == nullptr)
		return;

	LOGD << "CloseSession Called! " << pCallFunc << ", socket handle : " << m_Socket->native_handle();
	
	OnDisconnected();

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

}

void AsioSession::Reset()
{
	delete m_Socket;
	m_Socket = nullptr;

	delete m_Resolver;
	m_Resolver = nullptr;
}