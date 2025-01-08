#pragma once


enum class ServiceType : uint8
{
    Server,
    Client
};

using SessionMaker = std::function<std::shared_ptr<class AsioSession>(void)>;

class AsioAcceptor;

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
    boost::asio::io_context& iocontext;
    std::set<std::shared_ptr<class AsioSession>> m_Sessions;
    int32 m_SessionCount = 0;
    int32 m_MaxSessionCount = 0;

    short m_Port;

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

private:
    boost::asio::io_context& m_IoContext;
    std::shared_ptr<AsioAcceptor> m_Acceptor;

};


class AsioSession;

class AsioClientService : public AsioService
{
public:
    AsioClientService(boost::asio::io_context& iocontext, const std::string& host, short port, SessionMaker sessionmaker, int32 maxSessionCount = 1);

    virtual ~AsioClientService() = default;

    virtual bool Start() override;

private:
    void DoConnect();

private:
    tcp::resolver m_Resolver;
    tcp::socket m_Socket;
    std::string m_Host;
    short m_Port;
};