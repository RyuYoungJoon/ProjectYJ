#pragma once

class AsioService;

class AsioAcceptor : public std::enable_shared_from_this<AsioAcceptor>
{
public:
    AsioAcceptor(boost::asio::io_context* iocontext, string& port, std::shared_ptr<AsioService> service)
        : m_IoContext(iocontext), m_Acceptor(*iocontext, tcp::endpoint(tcp::v4(), stoi(port))), m_Service(service)
    {
    }

    virtual ~AsioAcceptor() = default;

    void Start();
    void Stop();
private:
    void DoAccept();
    void HandleAccept(tcp::socket* newSocket, boost::system::error_code ec);

private:
    boost::asio::io_context* m_IoContext;
    tcp::acceptor m_Acceptor;
    std::shared_ptr<AsioService> m_Service; // Reference to the parent service
};