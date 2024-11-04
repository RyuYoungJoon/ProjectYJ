#include "pch.h"
#include "AsioServer.h"

void AsioServer::DoAccept()
{
	auto aConnection = std::make_shared<tcp::socket>(m_Acceptor.get_executor().context());

	// Ŭ���̾�Ʈ �޾��ֱ�
	m_Acceptor.async_accept(*aConnection, [this, aConnection](const boost::system::error_code& error) {
		if (!error)
		{
			std::cout << "Connected!" << std::endl;
			DoRead(aConnection);	// �پƷ� ������ �б� ����
		}

		DoAccept(); // �ٸ� Ŭ���̾�Ʈ �޾ƿ���.
	});
}

void AsioServer::DoRead(std::shared_ptr<tcp::socket> socket)
{
	auto buffer = std::make_shared<std::vector<char>>(1024);

	// ������ �б�
	socket->async_read_some(boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& error, std::size_t transferBytes) {
		if (!error)
		{
			std::cout << "Recv Data : " << std::string(buffer->data(), transferBytes) << std::endl;
			DoWrite(socket, "Hi"); // �޾Ҵٰ� �����ϱ�.
			DoRead(socket);
		}
		else // �Ϸ� ���� ����
		{
			std::cerr << "Error!! Recv : " << error.message() << std::endl;
		}	
	});

}

void AsioServer::DoWrite(std::shared_ptr<tcp::socket> socket, const std::string& message)
{
	auto buffer = std::make_shared<std::string>(message);

	// ������ ������
	boost::asio::async_write(*socket, boost::asio::buffer(*buffer), [buffer](const boost::system::error_code& error, std::size_t) { // ������ ���� �ʿ�
		if (!error)
		{
			std::cout << "Message Send : " << *buffer << std::endl;
		}
		else // �Ϸ� ���� ����
		{
			std::cerr << "Error!! Send : " << error.message() << std::endl;
		}
	}); // buffer�� ���ϴ� ����?

}
