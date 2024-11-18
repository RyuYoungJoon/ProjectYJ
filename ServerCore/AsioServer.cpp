#include "pch.h"
#include "AsioServer.h"

void AsioServer::DoAccept()
{
	// 클라이언트 받아주기
	m_Acceptor.async_accept([this](const boost::system::error_code& error, tcp::socket socket) {
		if (!error)
		{
			// TODO : Session Count로 풀에 관리
			std::cout << "Connected!" << std::endl;
			std::make_shared<AsioSession>(std::move(socket))->Start();
		}
		else
		{
			std::cout << "Connected Fail!" << std::endl;
		}

		DoAccept(); // 다른 클라이언트 받아오자.
	});
}

AsioServer::~AsioServer()
{
	// TODO : 객체 소멸
}

//
//void AsioServer::Start()
//{
//	ioContext.run();
//}
//
//void AsioServer::HandlePacket(const std::string& packet)
//{
//	// TODO : 핸들러 만들기
//	int a;
//	a = 1;
//}
