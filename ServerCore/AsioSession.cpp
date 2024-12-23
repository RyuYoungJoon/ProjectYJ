#include "pch.h"
#include "AsioSession.h"

void AsioSession::Start()
{
	m_socket.async_read_some(boost::asio::buffer(data_, max_length),
		boost::bind(&AsioSession::DoRead, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void AsioSession::DoRead(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        boost::asio::async_write(m_socket,
            boost::asio::buffer(data_, bytes_transferred),
            boost::bind(&AsioSession::DoWrite, this,
                boost::asio::placeholders::error));
    }
    else
    {
        delete this;
    }
}

void AsioSession::DoWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        m_socket.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&AsioSession::DoRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
