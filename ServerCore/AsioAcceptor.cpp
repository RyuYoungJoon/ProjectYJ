#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "SocketUtil.h"

void AsioAcceptor::Start()
{
    if (m_Acceptor.is_open())
    {
        m_Acceptor.close();
    }

    tcp::endpoint endpoint(tcp::v4(), 7777);

    boost::system::error_code ec;

    m_Acceptor.open(endpoint.protocol(), ec);

    if (ec)
    {
        LOGE << "Acceptor open fail";
        return;
    }

    m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);

    if (ec)
    {
        LOGE << "SET_OPTION FAIL";
        return;
    }

    m_Acceptor.bind(endpoint, ec);

    if (ec)
    {
        LOGE << "BIND FAIL";
        return;
    }

    m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        LOGE << "Failed to listen on port " << 7777 << ": " << ec.message();
        return;
    }

    DoAccept();
}

void AsioAcceptor::Stop()
{
    if (m_Acceptor.is_open())
    {
        boost::system::error_code ec;
        m_Acceptor.close(ec);

        if (!ec)
        {
            LOGI << "Accept STOP!";
        }
    }
}

void AsioAcceptor::DoAccept()
{
    // ���ο� ���� ����
    // ���� Ǯ �����ұ�.
    auto newSocket = new tcp::socket(*m_IoContext);

   

    /*tcp::endpoint endPoint(tcp::v4(), 7777);
    newSocket->open(endPoint.protocol());
    boost::system::error_code ec;
    newSocket->set_option(boost::asio::socket_base::reuse_address(true), ec);*/
    /*SocketUtil<tcp> socketOption(newSocket);
    socketOption.SetReuseAddress(true);
    socketOption.SetLinger(false, 0);
    socketOption.SetKeepAlive(true);
    socketOption.SetNodelay(true);*/

    //newSocket->bind(endPoint);

    // �� ���� �ޱ�
    m_Acceptor.async_accept(*newSocket, std::bind(&AsioAcceptor::HandleAccept, this, newSocket, std::placeholders::_1));
}

void AsioAcceptor::HandleAccept(tcp::socket* newSocket, boost::system::error_code ec)
{
    if (!ec)
    {
        // ���� ����� ���� ��ŸƮ
        auto session = m_Service->CreateSession(m_IoContext, newSocket);
        
        session->ProcessRecv();
        LOGI << "New Client Connected [" << session->GetSessionUID() << "]";

        // �� ������ ����
        DoAccept();
    }
    else
    {
        LOGE << "Accept Error : " <<ec.value()<< ", message: " << ec.message();
    }
}
