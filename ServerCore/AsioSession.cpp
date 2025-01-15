#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"

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
    //std::lock_guard<mutex> Lock(m_Mutex);

    //std::vector<char> buffer;
    //buffer.resize(message.header.size);

    ////char* buff = nullptr;

    //std::memcpy(buffer.data(), &message.header, sizeof(PacketHeader));

    ////std::memcpy(&buff, &message.header, sizeof(PacketHeader));
    ////std::memcpy(&buff + sizeof(PacketHeader), message.payload, message.header.size - sizeof(PacketHeader));


    //std::memcpy(buffer.data() + sizeof(PacketHeader), message.payload, message.header.size - sizeof(PacketHeader));

    //boost::asio::async_write(m_Socket, boost::asio::buffer(buffer),
    //    std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);


        m_SendQueue.push(message); // 큐에 Packet을 직접 추가
    }

    //m_IsSending = true; // 전송 상태 플래그 설정
    

    Packet packet;
    
    {

        packet = m_SendQueue.front(); // 큐의 첫 번째 패킷을 꺼냄

    }
    // 패킷 직렬화
    //std::vector<BYTE> buffer(packet.header.size);
    std::array<BYTE, 1024> buffer;
    std::memcpy(buffer.data(), &packet.header, sizeof(PacketHeader));
    std::memcpy(buffer.data() + sizeof(PacketHeader), packet.payload, packet.header.size - sizeof(PacketHeader));

    auto self = shared_from_this();
    boost::asio::async_write(m_Socket, boost::asio::buffer(buffer),
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
            m_PacketBuffer.Read(m_RecvBuffer.data(), header.size);

            // Step 5: OnRecv 호출
            OnRecv(m_RecvBuffer.data(), static_cast<int32>(m_RecvBuffer.size()));
            m_RecvBuffer.fill(0);
        }

        // 다음 비동기 읽기 시작
        DoRead();
    }
    else if (ec == boost::asio::error::eof)
    {
        std::cerr << "Connection closed by peer." << std::endl;
        CloseSession();
    }
    else if (ec == boost::asio::error::operation_aborted)
    {
        std::cerr << "Operation aborted." << std::endl;
        CloseSession();
    }
    else
    {
        std::cerr << "Read error: " << ec.message() << " (code: " << ec.value() << ")" << std::endl;
        CloseSession();
    }
}

void AsioSession::HandleWrite(boost::system::error_code ec, std::size_t length)
{
    if (ec)
    {
        cout << "Session Close" << endl;

        //CloseSession();
    }
    else
    {
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_SendQueue.pop(); // 완료된 패킷을 제거
        
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
