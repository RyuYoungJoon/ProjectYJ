#pragma once
#include "pch.h"
#include "PacketBuffer.h"

class AsioService;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
    AsioSession();
    AsioSession(boost::asio::io_context* iocontext, tcp::socket* socket);
    virtual ~AsioSession();

    void InitSession(boost::asio::io_context* ioContext, tcp::socket* socket);

    void ProcessRecv();
    void Send(const Packet& message);
    bool Connect(const string& host, const string& port);
    void Disconnect();

    void SetService(std::shared_ptr<AsioService> service);
    shared_ptr<AsioService> GetService() { return m_Service.lock(); }

    tcp::socket& GetSocket() { return *m_Socket; }

    AsioSessionPtr GetSession()
    {
        return static_pointer_cast<AsioSession>(shared_from_this());
    }

    int32 GetSessionUID() { return m_SessionUID; }
    void SetSessionUID(int32 sessionUID) { m_SessionUID = sessionUID; }

    void CloseSession(const char* pCallFunc = nullptr);

    void SetIsRunning(bool isRunning) { m_IsRunning = isRunning; }
    bool GetIsRunning() { return m_IsRunning; }
    
    void Reset();

    void ProcessBufferData();
    int32 ProcessPacket(BYTE* buffer, int32 len);

    void UpdateBufferReadPosition(int32 processedLen);

    boost::asio::io_context* GetIocontext() { return m_IoContext; }

    virtual void OnSend(int32 len) { LOGD << "AsioSession"; }
protected:
    virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
    virtual void OnConnected() {}
    virtual void OnDisconnected() {}

    boost::asio::io_context* m_IoContext;
    tcp::socket* m_Socket;

    bool m_IsRunning = false;

private:
    void DoRead();
    void HandleRead(boost::system::error_code ec, int32 length);
    void HandleWrite(boost::system::error_code ec, int32 length, Packet* packet);

private:
    std::mutex m_Mutex;
    weak_ptr<AsioService> m_Service;
    std::atomic<int32> m_SessionUID;
    PacketBuffer m_PacketBuffer;
    tcp::resolver* m_Resolver;
    shared_ptr<boost::asio::steady_timer> m_Timer;
};