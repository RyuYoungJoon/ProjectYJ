#include "pch.h"
#include "AsioSession.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "Logger.h"

void AsioAcceptor::Start()
{
    DoAccept();
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

        m_Service->AddSession(session);

        cout << Logger::DLog("New Client Connected!") << endl;

        // �� ������ ����
        DoAccept();
    }
    else
    {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    }
}
