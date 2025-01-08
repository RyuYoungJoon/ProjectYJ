#pragma once

class AsioService;

class AsioAcceptor : public std::enable_shared_from_this<AsioAcceptor>
{
public:
    AsioAcceptor(boost::asio::io_context& iocontext, short port, std::shared_ptr<AsioService> service)
        : m_IoContext(iocontext), m_Acceptor(iocontext, tcp::endpoint(tcp::v4(), port)), m_Service(service)
    {
    }

    virtual ~AsioAcceptor() = default;

    void Start();
private:
    void DoAccept();
    void HandleAccept(std::shared_ptr<tcp::socket> newSocket, boost::system::error_code ec);

private:
    boost::asio::io_context& m_IoContext;
    tcp::acceptor m_Acceptor;
    std::shared_ptr<AsioService> m_Service; // Reference to the parent service
};