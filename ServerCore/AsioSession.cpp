#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "MemoryPoolManager.h"
#include "TaskQueue.h"


AsioSession::AsioSession(boost::asio::io_context& iocontext, tcp::socket socket)
    : m_IoContext(iocontext), m_Socket(std::move(socket)), m_PacketBuffer(65536)
{
}

void AsioSession::Start()
{
    m_SessionUID.fetch_add(1);

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
    boost::asio::async_write(m_Socket, boost::asio::buffer(buffer,bufferSize),
        std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::SetService(std::shared_ptr<AsioService> service)
{
    m_Service = service;
}

void AsioSession::DoRead()
{
    std::lock_guard<mutex> Lock(m_Mutex);
    
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
        int32 processLen = OnRecvPacket(m_PacketBuffer.ReadPos(), dataSize);
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

        //CloseSession();
    }
}

int32 AsioSession::OnRecvPacket(BYTE* buffer, int32 len)
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
    m_Socket.close();
    if (m_Service)
    {
        m_Service->ReleaseSession(shared_from_this());
    }
}
