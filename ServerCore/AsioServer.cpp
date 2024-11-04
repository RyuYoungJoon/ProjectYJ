#include "pch.h"
#include "AsioServer.h"

void AsioServer::DoAccept()
{
	auto aConnection = std::make_shared<tcp::socket>(m_Acceptor.get_executor().context());

	// 클라이언트 받아주기
	m_Acceptor.async_accept(*aConnection, [this, aConnection](const boost::system::error_code& error) {
		if (!error)
		{
			std::cout << "Connected!" << std::endl;
			DoRead(aConnection);	// 바아로 데이터 읽기 시작
		}

		DoAccept(); // 다른 클라이언트 받아오자.
	});
}

void AsioServer::DoRead(std::shared_ptr<tcp::socket> socket)
{
	auto buffer = std::make_shared<std::vector<char>>(1024);

	// 데이터 읽기
	socket->async_read_some(boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& error, std::size_t transferBytes) {
		if (!error)
		{
			std::cout << "Recv Data : " << std::string(buffer->data(), transferBytes) << std::endl;
			DoWrite(socket, "Hi"); // 받았다고 전달하기.
			DoRead(socket);
		}
		else // 일루 오면 에러
		{
			std::cerr << "Error!! Recv : " << error.message() << std::endl;
		}	
	});

}

void AsioServer::DoWrite(std::shared_ptr<tcp::socket> socket, const std::string& message)
{
	auto buffer = std::make_shared<std::string>(message);

	// 데이터 보내기
	boost::asio::async_write(*socket, boost::asio::buffer(*buffer), [buffer](const boost::system::error_code& error, std::size_t) { // 사이즈 개발 필요
		if (!error)
		{
			std::cout << "Message Send : " << *buffer << std::endl;
		}
		else // 일루 오면 에러
		{
			std::cerr << "Error!! Send : " << error.message() << std::endl;
		}
	}); // buffer은 뭐하는 애지?

}
