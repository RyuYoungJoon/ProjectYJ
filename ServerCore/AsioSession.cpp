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
	// TODO : ��Ŷ �ڵ�, ����� �ٵ� �д� �ڵ� ����
	
	auto self(shared_from_this());

	// ������ �б�
	m_socket.async_read_some(boost::asio::buffer(readBuffer), [this, self](const boost::system::error_code& error, std::size_t transferBytes) {
		if (!error)
		{
			std::cout << "Recv Data : " << std::string(readBuffer.data(), transferBytes) << std::endl;
			DoWrite(); // �޾Ҵٰ� �����ϱ�.
			DoRead();
		}
		else // �Ϸ� ���� ����
		{
			std::cerr << "Error!! Recv : " << error.message() << std::endl;
		}
		});

}

void AsioSession::DoWrite()
{
	auto self(shared_from_this());
	// ������ ������
	boost::asio::async_write(m_socket, boost::asio::buffer(writeBuffer.front().data(), writeBuffer.front().length()), [this, self](const boost::system::error_code& error, std::size_t) { // ������ ���� �ʿ�
		if (!error)
		{
			writeBuffer.pop();
			if (!writeBuffer.empty())
			{
				DoWrite();
			}
		}
		else // �Ϸ� ���� ����
		{
			std::cerr << "Error!! Send : " << error.message() << std::endl;
		}
		});
}