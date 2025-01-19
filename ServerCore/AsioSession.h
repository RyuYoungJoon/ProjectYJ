#pragma once
#include "pch.h"
#include "PacketBuffer.h"
#include "..\GameServer\Protocol.h"

class AsioService;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
    AsioSession(boost::asio::io_context& iocontext, tcp::socket socket);
    
    virtual ~AsioSession()
    {
    }

    void Start();
    void Send(const Packet& message);

    void SetService(std::shared_ptr<AsioService> service);
    tcp::socket& GetSocket() { return m_Socket; }

    std::shared_ptr<AsioSession> GetSession()
    {
        return static_pointer_cast<AsioSession>(shared_from_this());
    }

    int32 GetSessionUID() { return m_SessionUID; }

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
    std::mutex m_Mutex;
    boost::asio::io_context& m_IoContext;
    tcp::socket m_Socket;
    std::array<char, 1024> m_ReadBuffer;
    std::array<BYTE, 4096> m_RecvBuffer;
    std::shared_ptr<AsioService> m_Service;
    std::atomic<int32> m_SessionUID;

protected:
    //std::queue<Packet> m_SendQueue;
    PacketBuffer m_PacketBuffer;
};
