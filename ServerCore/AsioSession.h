#pragma once
#include "pch.h"
#include "PacketBuffer.h"
#include "TaskQueue.h"
#include "ObjectPool.h"

class AsioService;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
    AsioSession();
    AsioSession(boost::asio::io_context* iocontext, tcp::socket* socket);
    virtual ~AsioSession();

    void InitSession(boost::asio::io_context* ioContext, tcp::socket* socket);

    void ProcessRecv();
    void ProcessDisconnect(const char* pCallback);
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

    int32 ProcessPacket(BYTE* buffer, int32 len);
    NetState GetNetState() const { return m_NetState; }
    void SetNetState(NetState netState) { m_NetState = netState; }

    boost::asio::io_context* GetIocontext() { return m_IoContext; }

    template<typename TPacket>
    void Send(const TPacket& packet)
    {
        static std::atomic<uint32> seqNum(0);

        TPacket* sendPacket = PacketPool::GetInstance().Pop<TPacket>();
        *sendPacket = packet;

        sendPacket->header.seqNum = seqNum.fetch_add(1);

        std::memset(sendPacket->header.checkSum, 0x12, sizeof(sendPacket->header.checkSum));
        std::memset(sendPacket->header.checkSum + 1, 0x34, sizeof(sendPacket->header.checkSum) - 1);

        sendPacket->header.size = static_cast<uint32>(sizeof(TPacket));

        sendPacket->tail.value = 255;

        m_Socket->async_write_some(boost::asio::buffer(sendPacket, sendPacket->header.size),
            std::bind(&AsioSession::HandleWrite<TPacket>, shared_from_this(), std::placeholders::_1, std::placeholders::_2, sendPacket));
    }

protected:
    virtual void OnSend(int32 len) { }
    virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
    virtual void OnConnected() {}
    virtual void OnDisconnected() {}

    boost::asio::io_context* m_IoContext;
    tcp::socket* m_Socket;

    bool m_IsRunning = false;

private:
    void DoRead();
    void HandleRead(boost::system::error_code ec, int32 length);

    template<typename TPacket>
    void HandleWrite(boost::system::error_code ec, int32 length, TPacket* packet)
    {
        if (ec)
        {
            LOGE << "Session Close : " << ec.value() << ", Message : " << ec.message();

            ProcessDisconnect(__FUNCTION__);
        }
        else
        {
            OnSend(length);
        }

        PacketPool::GetInstance().Push(packet);
    }


private:
    std::mutex m_Mutex;
    weak_ptr<AsioService> m_Service;
    std::atomic<int32> m_SessionUID;
    PacketBuffer m_PacketBuffer;
    tcp::resolver* m_Resolver;
    shared_ptr<boost::asio::steady_timer> m_Timer;
    NetState m_NetState = NetState::None;
};