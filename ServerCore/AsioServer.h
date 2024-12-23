#pragma once
#include "AsioSession.h"
#include "AsioIoContext.h"

using boost::asio::ip::tcp;


class AsioServer
{
public:
    AsioServer(boost::asio::io_context& iocontext, short port)
        : m_IoContext(iocontext),
        m_Acceptor(iocontext, tcp::endpoint(tcp::v4(), port))
    {
        Listen();
    }

private:
    void Listen();

    void DoAccept(AsioSession* new_session, const boost::system::error_code& error);

    boost::asio::io_context& m_IoContext;
    tcp::acceptor m_Acceptor;
};