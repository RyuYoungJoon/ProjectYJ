#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "MemoryPoolManager.h"
#include "TaskQueue.h"

extern int totalCnt;
extern int RealTryCnt;

AsioSession::AsioSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: m_IoContext(iocontext), m_Socket(std::move(socket)), m_PacketBuffer(65536), m_Resolver(iocontext)
{
}

void AsioSession::Start()
{
	DoRead();
}

void AsioSession::Send(const Packet& message)
{
	// 메모리풀 매니저에서 메모리를 가져오자.
	size_t bufferSize = message.header.size;

	// 메모리풀에 버퍼 사이즈만큼 메모리를 가져와서 buffer 생성
	BYTE* buffer = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(bufferSize).Allocate());

	std::memcpy(buffer, &message.header, sizeof(PacketHeader));
	std::memcpy(buffer + sizeof(PacketHeader), message.payload, message.header.size - sizeof(PacketHeader));

	// TODO : 메모리 DeAllocate 구조잡기
	auto self = shared_from_this();
	m_Socket.async_write_some(boost::asio::mutable_buffer(buffer, bufferSize),
		std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	//MemoryPoolManager::GetMemoryPool(bufferSize).Deallocate(buffer);
}

bool AsioSession::Connect(const string& host, const string& port)
{
	tcp::resolver::query targetQuery(host, port);
	auto targetEndpoint = m_Resolver.resolve(targetQuery);

	// TODO Cnt 위치 옮기기
	auto self = shared_from_this();
	boost::asio::async_connect(m_Socket, targetEndpoint,
		[this, self](boost::system::error_code ec, tcp::endpoint endpoint)
		{
			if (!ec)
			{
				LOGI << "Successfully connected to " << endpoint;
				Start();

				int random = OnConnected();
				totalTryCnt += random;

				LOGI << "TotalCnt[" << totalCnt << "] totalTryCnt[" << totalTryCnt << "]";
				
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
	CloseSession();
	OnDisconnected();
}

void AsioSession::SetService(std::shared_ptr<AsioService> service)
{
	m_Service = service;
}

void AsioSession::DoRead()
{
	m_Socket.async_read_some(boost::asio::buffer(m_PacketBuffer.WritePos(), m_PacketBuffer.FreeSize()),
		std::bind(&AsioSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::HandleRead(boost::system::error_code ec, std::size_t length)
{
	if (!ec)
	{
		if (m_PacketBuffer.OnWrite(length) == false)
		{
			LOGE << "OnWrite OverFlow";
			CloseSession();
			return;
		}

		int32 dataSize = m_PacketBuffer.DataSize();

		// 이걸 작업 큐에?
		int32 processLen = ProcessPacket(m_PacketBuffer.ReadPos(), dataSize);
		if (processLen < 0 || dataSize < processLen || m_PacketBuffer.OnRead(processLen) == false)
		{
			LOGE << "OnRead OverFlow";
			CloseSession();
			return;
		}

		m_PacketBuffer.Clear();

		// 다음 비동기 읽기 시작
		DoRead();
	}
	else if (ec == boost::asio::error::eof)
	{
		LOGE << "Connection closed by peer" << endl;
		CloseSession();
	}
	else if (ec == boost::asio::error::operation_aborted)
	{
		LOGE << "Operation aborted.";
		CloseSession();
	}
	else
	{
		if (ec.value() == boost::asio::error::connection_reset)
			LOGE << "CloseSession";
		else
			LOGE << "Read error : " << ec.message() << " (code : " << ec.value() << ")";

		CloseSession();
	}
}

void AsioSession::HandleWrite(boost::system::error_code ec, std::size_t length)
{
	if (ec)
	{
		LOGE << "Session Close";

		CloseSession();
	}   
	else
	{
		// 실제로 보낸 횟수
		RealTryCnt++;
		Disconnect();
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

		processLen += header.size;
	}

	return processLen;
}

void AsioSession::CloseSession()
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	if (!m_Socket.is_open())
		return;

	boost::asio::post(m_IoContext, [self = shared_from_this()]() {
		if (self->m_Socket.is_open())
		{
			self->m_Socket.close();
			LOGI << "SAFE";
		}
		});

	/*if (auto service = m_Service.lock())
	{
		service->ReleaseSession(shared_from_this());
	}*/
}
