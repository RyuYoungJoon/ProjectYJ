#include "pch.h"
#include "AsioServer.h"

void AsioServer::DoAccept()
{
	// Ŭ���̾�Ʈ �޾��ֱ�
	m_Acceptor.async_accept([this](const boost::system::error_code& error, tcp::socket socket) {
		if (!error)
		{
			// TODO : Session Count�� Ǯ�� ����
			std::cout << "Connected!" << std::endl;
			std::make_shared<AsioSession>(std::move(socket))->Start();
		}
		else
		{
			std::cout << "Connected Fail!" << std::endl;
		}

		DoAccept(); // �ٸ� Ŭ���̾�Ʈ �޾ƿ���.
	});
}

AsioServer::~AsioServer()
{
	// TODO : ��ü �Ҹ�
}

//
//void AsioServer::Start()
//{
//	ioContext.run();
//}
//
//void AsioServer::HandlePacket(const std::string& packet)
//{
//	// TODO : �ڵ鷯 �����
//	int a;
//	a = 1;
//}
