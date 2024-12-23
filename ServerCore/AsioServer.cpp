#include "pch.h"
#include "AsioServer.h"

void AsioServer::Listen()
{
	// 서버가 Listen하고 Session을 생성한다.
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
        // 에러 발생하면 제거!
        delete new_session;
    }

    // 서버 Listen
    Listen();
}
