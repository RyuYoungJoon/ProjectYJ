#pragma once
#include "ObjectPool.h"

enum class ServiceType : uint8
{
    Server,
    Client
};

class AsioSession;
class AsioAcceptor;

using SessionMaker = std::function<AsioSessionPtr(boost::asio::io_context*, tcp::socket*)>;

class AsioService : public std::enable_shared_from_this<AsioService>
{
public:
    AsioService(ServiceType type, boost::asio::io_context* iocontext, string& host, string& port ,SessionMaker SessionMaker, int32 maxSessionCount = 1);
    virtual ~AsioService();

    virtual bool Start();
    bool CanStart() { return m_SessionMaker != nullptr; }

    virtual void CloseService();

    AsioSessionPtr CreateSession(boost::asio::io_context* iocontext, tcp::socket* socket);
    void AddSession(AsioSessionPtr session);
    void ReleaseSession(AsioSessionPtr session);
    void BroadCast(Packet& packet);

    int32 GetMaxSessionCount() { return m_MaxSessionCount; }
    
    size_t GetSessionSize() { return m_Sessions.size(); }

    boost::asio::io_context* iocontext;
public:
    ServiceType GetServiceType() { return m_type; }

protected:
    ServiceType m_type;
    std::set<std::shared_ptr<class AsioSession>> m_Sessions;
    int32 m_SessionCount = 0;
    int32 m_MaxSessionCount = 0;
    std::atomic<int32> m_SessionUID = 0;
    std::mutex m_Mutex;

    std::string m_Host;
    std::string m_Port;

    SessionMaker m_SessionMaker;
    bool m_checkRunning = false;
};


class AsioServerService : public AsioService
{
public:
    AsioServerService(boost::asio::io_context* iocontext, string& host, string& port, SessionMaker SessionMaker, int32 maxSessionCount = 1);
    virtual ~AsioServerService();

public:
    virtual bool Start() override;
    virtual void CloseService() override;

private:
    boost::asio::io_context* m_IoContext;
    std::shared_ptr<AsioAcceptor> m_Acceptor;

};


class AsioClientService : public AsioService
{
public:
    AsioClientService(boost::asio::io_context* iocontext, string& host, string& port, SessionMaker SessionMaker, int32 maxSessionCount = 1);

    virtual ~AsioClientService();

    virtual bool Start() override;
private:
    tcp::socket* m_Socket;
};