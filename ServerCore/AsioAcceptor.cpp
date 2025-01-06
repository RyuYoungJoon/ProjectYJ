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
    // 새로운 소켓 생성
    auto newSocket = std::make_shared<tcp::socket>(m_IoContext);

    // 새 연결 받기
    m_Acceptor.async_accept(*newSocket, std::bind(&AsioAcceptor::HandleAccept, shared_from_this(), std::placeholders::_1));
}

void AsioAcceptor::HandleAccept(boost::system::error_code ec)
{
    if (!ec)
    {
        // 세션 만들고 세션 스타트
        auto session = m_Service->CreateSession();
        session->Start();

        std::cout << "New client connected!" << std::endl;

        // 또 받으러 가기
        DoAccept();
    }
    else
    {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    }
}
