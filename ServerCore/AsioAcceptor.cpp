#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "AsioAcceptor.h"

void AsioAcceptor::Start()
{
    if (m_Acceptor.is_open())
    {
        m_Acceptor.close();
    }

    // Acceptor ����
    m_Acceptor.open(tcp::v4());
    
    // ���� �ɼ� ����
    boost::asio::socket_base::reuse_address reuseAddrOpt(true);
    boost::asio::socket_base::linger lingerOpt(true, 0);
    boost::asio::ip::tcp::no_delay noDelayOpt(true);
    boost::asio::socket_base::receive_buffer_size recvBufferSizeOpt(65536);
    boost::asio::socket_base::send_buffer_size sendBufferSizeOpt(65536);
    boost::asio::socket_base::keep_alive keepAliveOpt(true);

    m_Acceptor.set_option(reuseAddrOpt);
    m_Acceptor.set_option(lingerOpt);
    m_Acceptor.set_option(noDelayOpt);
    m_Acceptor.set_option(recvBufferSizeOpt);
    m_Acceptor.set_option(sendBufferSizeOpt);
    m_Acceptor.set_option(keepAliveOpt);

    m_Acceptor.bind(tcp::endpoint(tcp::v4(), 7777));
    m_Acceptor.listen(65536);

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
    auto newSocket = std::make_shared<tcp::socket>(m_IoContext);
    // �� ���� �ޱ�
    m_Acceptor.async_accept(*newSocket, std::bind(&AsioAcceptor::HandleAccept, this, newSocket, std::placeholders::_1));
}

void AsioAcceptor::HandleAccept(std::shared_ptr<tcp::socket> newSocket, boost::system::error_code ec)
{
    if (!ec)
    {
        // ���� ����� ���� ��ŸƮ
        auto session = m_Service->CreateSession(m_IoContext, std::move(*newSocket));

        session->Start();
        session->SetSessionUID(m_User.fetch_add(1));

        m_Service->AddSession(session);
        LOGI << "New Client Connected [" << session->GetSessionUID() << "]";

        // �� ������ ����
        DoAccept();
    }
    else
    {
        LOGE << "Accept Error : " <<ec.value()<< ", message: " << ec.message();
    }
}
