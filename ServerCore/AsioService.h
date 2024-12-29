#pragma once

enum class ServiceType : uint8
{
    Server,
    Client
};

using SessionMaker = std::function<std::shared_ptr<class AsioSession>(void)>;

class AsioService : public std::enable_shared_from_this<AsioService>
{
public:
    AsioService(ServiceType type, boost::asio::io_context& iocontext, short port ,SessionMaker sessionmaker, int32 maxSessionCount = 1);
    virtual ~AsioService();

    virtual bool Start() abstract;
    bool CanStart();

    virtual void CloseService();

    std::shared_ptr<class AsioSession> CreateSession();
    void AddSession(std::shared_ptr<class AsioSession> session);
    void ReleaseSession(std::shared_ptr<class AsioSession> session);

public:
    ServiceType GetServiceType() { return m_type; }

protected:
    ServiceType m_type;

    std::set<std::shared_ptr<class AsioSession>> m_Sessions;
    int32 m_SessionCount = 0;
    int32 m_MaxSessionCount = 0;

    SessionMaker m_SessionMaker;
};


class AsioServerService : public AsioService
{
public:
    AsioServerService(boost::asio::io_context& iocontext, short port, SessionMaker sessionmaker, int32 maxSessionCount = 1);
    virtual ~AsioServerService();

public:
    virtual bool Start() override;
    virtual void CloseService() override;
    void Listen();

    void DoAccept(AsioSession* new_session, const boost::system::error_code& error);


private:
    boost::asio::io_context& m_IoContext;
    tcp::acceptor m_Acceptor;
};

class AsioClientService : public AsioService
{
public:
    AsioClientService(boost::asio::io_context& iocontext, short port, SessionMaker sessionmaker, int32 maxSessionCount = 1);
    virtual ~AsioClientService();

    virtual bool Start() override;
};