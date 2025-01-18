#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "MemoryPoolManager.h"
#include "Logger.h"

AsioSession::AsioSession(boost::asio::io_context& iocontext, tcp::socket socket)
    : m_IoContext(iocontext), m_Socket(std::move(socket)), m_PacketBuffer(4096)
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

    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer),
        std::bind(&AsioSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::HandleRead(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        m_PacketBuffer.Write(m_ReadBuffer.data(), length);

        while (m_PacketBuffer.ReadableSize() >= sizeof(PacketHeader))
        {
            // Step 1: 헤더 읽기
            PacketHeader header;
            m_PacketBuffer.Peek(&header, sizeof(PacketHeader));

            // Step 2: 유효성 검사
            /*if (header.checkSum != '0x1234')
            {
                std::cerr << "Invalid Packet: CheckValue mismatch." << std::endl;
                m_PacketBuffer.DiscardReadData();
                break;
            }*/

            // Step 3: 패킷 전체 크기 확인
            if (m_PacketBuffer.ReadableSize() < header.size)
                break; // 패킷 전체가 도착하지 않음

            // Step 4: 패킷 데이터 읽기
            size_t packetSize = header.size;
            BYTE* packetData = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(packetSize).Allocate());
            m_PacketBuffer.Read(packetData, packetSize);

            // Step 5: OnRecv 호출
            OnRecv(packetData, static_cast<int32>(m_RecvBuffer.size()));
            MemoryPoolManager::GetMemoryPool(packetSize).Deallocate(packetData);


            // Step 6: 버퍼 초기화
            m_PacketBuffer.DiscardReadData();
        }

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

void AsioSession::CloseSession()
{
    m_Socket.close();
    if (m_Service)
    {
        m_Service->ReleaseSession(shared_from_this());
    }
}
