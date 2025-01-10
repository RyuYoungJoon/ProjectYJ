#pragma once

#include "pch.h"
#include "PacketBuffer.h"

class AsioService;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
    AsioSession(boost::asio::io_context& iocontext, tcp::socket socket)
        : m_IoContext(iocontext), m_Socket(std::move(socket)), m_PacketBuffer(4096)
    {
    }

    virtual ~AsioSession()
    {
    }

    void Start();
    void Send(const std::string& message);

    void SetService(std::shared_ptr<AsioService> service);
    tcp::socket& GetSocket() { return m_Socket; }

protected:
    virtual void OnSend(int32 len) { }
    virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
    virtual void OnConnected() { }
    virtual void OnDisconnected() { }

private:
    void DoRead();

    void HandleRead(boost::system::error_code ec, std::size_t length);

    void HandleWrite(boost::system::error_code ec, std::size_t length);

    void CloseSession();

private:
    boost::asio::io_context& m_IoContext;
    tcp::socket m_Socket;
    std::array<char, 1024> m_ReadBuffer;
    PacketBuffer m_PacketBuffer;
    std::shared_ptr<AsioService> m_Service;
};
