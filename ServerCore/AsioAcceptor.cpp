#include "pch.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
#include "AsioService.h"

AsioAcceptor::AsioAcceptor()
{
}

AsioAcceptor::~AsioAcceptor()
{
}

bool AsioAcceptor::StartAccept(std::shared_ptr<AsioServerService> service)
{
	std::shared_ptr<AsioSession> session = service->CreateSession();

	m_Acceptor.async_accept(session->GetSocket(),
		boost::bind(&AsioAcceptor::DoAccept, this, session,
			boost::asio::placeholders::error));
}

void AsioAcceptor::CloseSocket()
{
}

void AsioAcceptor::DoAccept(AsioSession* session, const boost::system::error_code& error)
{
    if (!error)
    {
        session->Start();
    }
    else
    {
        // 에러 발생하면 제거!
        delete session;
    }

}
