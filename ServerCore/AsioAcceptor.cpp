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
    // ���ο� ���� ����
    auto newSocket = std::make_shared<tcp::socket>(m_IoContext);

    // �� ���� �ޱ�
    m_Acceptor.async_accept(*newSocket, std::bind(&AsioAcceptor::HandleAccept, shared_from_this(), std::placeholders::_1));
}

void AsioAcceptor::HandleAccept(boost::system::error_code ec)
{
    if (!ec)
    {
        // ���� ����� ���� ��ŸƮ
        auto session = m_Service->CreateSession();
        session->Start();

        std::cout << "New client connected!" << std::endl;

        // �� ������ ����
        DoAccept();
    }
    else
    {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    }
}
