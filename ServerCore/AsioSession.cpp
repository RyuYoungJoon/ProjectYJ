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
    // �޸�Ǯ �Ŵ������� �޸𸮸� ��������.
    size_t bufferSize = message.header.size;

    // �޸�Ǯ�� ���� �����ŭ �޸𸮸� �����ͼ� buffer ����
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
            // Step 1: ��� �б�
            PacketHeader header;
            m_PacketBuffer.Peek(&header, sizeof(PacketHeader));

            // Step 2: ��ȿ�� �˻�
            /*if (header.checkSum != '0x1234')
            {
                std::cerr << "Invalid Packet: CheckValue mismatch." << std::endl;
                m_PacketBuffer.DiscardReadData();
                break;
            }*/

            // Step 3: ��Ŷ ��ü ũ�� Ȯ��
            if (m_PacketBuffer.ReadableSize() < header.size)
                break; // ��Ŷ ��ü�� �������� ����

            // Step 4: ��Ŷ ������ �б�
            size_t packetSize = header.size;
            BYTE* packetData = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(packetSize).Allocate());
            m_PacketBuffer.Read(packetData, packetSize);

            // Step 5: OnRecv ȣ��
            OnRecv(packetData, static_cast<int32>(m_RecvBuffer.size()));
            MemoryPoolManager::GetMemoryPool(packetSize).Deallocate(packetData);


            // Step 6: ���� �ʱ�ȭ
            m_PacketBuffer.DiscardReadData();
        }

        // ���� �񵿱� �б� ����
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
