#include "pch.h"
#include "AsioServer.h"

void AsioServer::Listen()
{
	// ������ Listen�ϰ� Session�� �����Ѵ�.
	AsioSession* NewSession = new AsioSession(m_IoContext);

	m_Acceptor.async_accept(NewSession->socket(),
		boost::bind(&AsioServer::DoAccept, this, NewSession,
			boost::asio::placeholders::error));
}

void AsioServer::DoAccept(AsioSession* new_session, const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->Start();
    }
    else
    {
        // ���� �߻��ϸ� ����!
        delete new_session;
    }

    // ���� Listen
    Listen();
}
