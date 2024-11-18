#include "pch.h"
#include "AsioSession.h"

AsioSession::~AsioSession()
{
	
}

void AsioSession::Start()
{
	DoRead();
}

void AsioSession::Send(const std::string& message)
{
	bool writeProgress = !writeBuffer.empty();
	writeBuffer.push(message);
	if (!writeProgress)
	{
		DoWrite();
	}
}

void AsioSession::DoRead()
{
	// TODO : 패킷 핸들, 헤더와 바디 읽는 코드 구현
	
	auto self(shared_from_this());

	// 데이터 읽기
	m_socket.async_read_some(boost::asio::buffer(readBuffer), [this, self](const boost::system::error_code& error, std::size_t transferBytes) {
		if (!error)
		{
			std::cout << "Recv Data : " << std::string(readBuffer.data(), transferBytes) << std::endl;
			DoWrite(); // 받았다고 전달하기.
			DoRead();
		}
		else // 일루 오면 에러
		{
			std::cerr << "Error!! Recv : " << error.message() << std::endl;
		}
		});

}

void AsioSession::DoWrite()
{
	auto self(shared_from_this());
	// 데이터 보내기
	boost::asio::async_write(m_socket, boost::asio::buffer(writeBuffer.front().data(), writeBuffer.front().length()), [this, self](const boost::system::error_code& error, std::size_t) { // 사이즈 개발 필요
		if (!error)
		{
			writeBuffer.pop();
			if (!writeBuffer.empty())
			{
				DoWrite();
			}
		}
		else // 일루 오면 에러
		{
			std::cerr << "Error!! Send : " << error.message() << std::endl;
		}
		});
}