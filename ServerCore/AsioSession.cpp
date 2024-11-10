#include "pch.h"
#include "AsioSession.h"

void AsioSession::Send(const std::string& message)
{
	auto buffer = std::make_shared<std::string>(message);
	boost::asio::async_write(m_socket, boost::asio::buffer(*buffer), [buffer](const boost::system::error_code& error, std::size_t) {
		if (error)
		{
			std::cerr << "Send Failed : " << error.message() << std::endl;
		}
	}
	);

}

void AsioSession::Recv()
{
	auto buffer = std::make_shared<std::vector<char>>(1024); // 사이즈 변경 예정
	m_socket.async_read_some(boost::asio::buffer(*buffer), [this, buffer](const boost::system::error_code& error, std::size_t transferredBytes) {
		if (!error)
		{
			std::cout << "Received data: " << std::string(buffer->data(), transferredBytes) << std::endl;
			Recv();
		}
		else
		{
			std::cerr << "Receive failed: " << error.message() << std::endl;
		}
	});

}
