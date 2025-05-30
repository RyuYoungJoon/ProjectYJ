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

    SocketUtil<tcp> socketOption(newSocket);
    socketOption.SetReuseAddress(true);
    socketOption.SetLinger(false, 0);
    socketOption.SetKeepAlive(true);
    socketOption.SetNodelay(true);

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
