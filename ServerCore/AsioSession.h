#pragma once


class AsioSession
{
    friend class AsioService;

public:
    AsioSession(boost::asio::io_context& iocontext);

    tcp::socket& GetSocket() { return m_socket; }

    void Start();

public:
    void DoRead(const boost::system::error_code& error, size_t bytes_transferred);
    
    void DoWrite(const boost::system::error_code& error);

public:
    void SetService(shared_ptr<AsioService> service) { m_service = service; }
    shared_ptr<AsioService> GetService() { return m_service.lock(); }

private:
    tcp::socket m_socket;
    weak_ptr<AsioService> m_service;

    enum { max_length = 1024 };
    char m_data[max_length]{};
};
