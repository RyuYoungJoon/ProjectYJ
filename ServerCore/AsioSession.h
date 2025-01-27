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
    bool Connect(const string& host, const string& port);
    void Disconnect();

    void SetService(std::shared_ptr<AsioService> service);
    shared_ptr<AsioService> GetService() { return m_Service.lock(); }

    tcp::socket& GetSocket() { return m_Socket; }

    AsioSessionPtr GetSession()
    {
        return static_pointer_cast<AsioSession>(shared_from_this());
    }

    int32 GetSessionUID() { return m_SessionUID; }
    void SetSessionUID(int32 sessionUID) { m_SessionUID = sessionUID; }

    void CloseSession();

protected:
    virtual void OnSend(int32 len) { }
    virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
    virtual void OnConnected() { }
    virtual void OnDisconnected() { }

private:
    void DoRead();
    void HandleRead(boost::system::error_code ec, std::size_t length);
    void HandleWrite(boost::system::error_code ec, std::size_t length);
    int32 ProcessPacket(BYTE* buffer, int32 len);
    

private:
    std::mutex m_Mutex;
    boost::asio::io_context& m_IoContext;
    tcp::socket m_Socket;
    weak_ptr<AsioService> m_Service;
    std::atomic<int32> m_SessionUID;
    PacketBuffer m_PacketBuffer;
    tcp::resolver m_Resolver;
};
