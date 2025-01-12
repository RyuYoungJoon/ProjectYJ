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
    std::vector<char> buffer;
    buffer.resize(message.header.size);

    std::memcpy(buffer.data(), &message.header, sizeof(PacketHeader));

    std::memcpy(buffer.data() + sizeof(PacketHeader), message.payload, message.header.size - sizeof(PacketHeader));

    boost::asio::async_write(m_Socket, boost::asio::buffer(buffer),
        std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::SetService(std::shared_ptr<AsioService> service)
{
    m_Service = service;
}

void AsioSession::DoRead()
{
    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer),
        std::bind(&AsioSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::HandleRead(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        // ������ �Ľ�
        if (!ec)
        {
            // ���ŵ� �����͸� PacketBuffer�� ���
            m_PacketBuffer.Write(m_ReadBuffer.data(), length);

            // ��Ŷ ó��
            while (m_PacketBuffer.ReadableSize() >= sizeof(PacketHeader))
            {
                PacketHeader header;
                m_PacketBuffer.Peek(&header, sizeof(PacketHeader));

                if (m_PacketBuffer.ReadableSize() < header.size)
                {
                    // ��ü ��Ŷ�� ���� �������� ����
                    break;
                }

                // ������ ��Ŷ�� ����
                Packet packet;
                m_PacketBuffer.Read(&packet.header, sizeof(PacketHeader));
                m_PacketBuffer.Read(packet.payload, header.size - sizeof(PacketHeader));

                // OnRecv ȣ��
                OnRecv(packet);
            }

            // ���� �����͸� ����
            m_PacketBuffer.DiscardReadData();

            // ���� �б� �۾� ����
            DoRead();
        }
        }
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
        CloseSession();
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
