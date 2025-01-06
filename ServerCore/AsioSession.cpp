#include "pch.h"
#include "AsioSession.h"

void AsioSession::Start()
{
    DoRead();
}

void AsioSession::Send(const std::string& message)
{
    auto self(shared_from_this());
    boost::asio::async_write(m_Socket, boost::asio::buffer(message),
        std::bind(&AsioSession::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::SetService(std::shared_ptr<AsioService> service)
{
    m_Service = service;
}

void AsioSession::DoRead()
{
    auto self(shared_from_this());
    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer),
        std::bind(&AsioSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioSession::HandleRead(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        m_PacketBuffer.Write(m_ReadBuffer.data(), length);

        // Process packets from the buffer
        while (m_PacketBuffer.ReadableSize() > 0)
        {
            // Example: Read a fixed-size packet (e.g., 4 bytes for testing)
            char packet[4];
            if (m_PacketBuffer.ReadableSize() >= sizeof(packet))
            {
                m_PacketBuffer.Read(packet, sizeof(packet));
                std::string message(packet, sizeof(packet));
                std::cout << "Received Packet: " << message << std::endl;
            }
            else
            {
                break;
            }
        }

        // Continue reading
        DoRead();
    }
    else
    {
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
