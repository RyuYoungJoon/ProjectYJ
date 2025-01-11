#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "AsioAcceptor.h"

void AsioAcceptor::Start()
{
    DoAccept();
}

void AsioAcceptor::DoAccept()
{

    // �̹� ���� �ִ� ��� �ݱ�
    if (m_Acceptor.is_open())
    {
        m_Acceptor.close();
    }

    m_Acceptor.open(tcp::v4());
    m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    m_Acceptor.bind(tcp::endpoint(tcp::v4(), 27931));
    m_Acceptor.listen();

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

        m_Service->AddSession(session);

        std::cout << "New client connected!" << std::endl;

        // �� ������ ����
        DoAccept();
    }
    else
    {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    }
}
