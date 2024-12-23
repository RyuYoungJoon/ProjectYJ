#pragma once

using boost::asio::ip::tcp;

class AsioSession
{
public:
    AsioSession(boost::asio::io_context& iocontext)
        : m_socket(iocontext)
    {
    }

    tcp::socket& socket()
    {
        return m_socket;
    }

    void Start();

private:
    void DoRead(const boost::system::error_code& error, size_t bytes_transferred);

    void DoWrite(const boost::system::error_code& error);

    tcp::socket m_socket;

    enum { max_length = 1024 };
    char data_[max_length];
};
