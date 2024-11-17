#include "pch.h"
#include "AsioSession.h"

AsioSession::~AsioSession()
{
	
}

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

// PacketSession
PacketSession::~PacketSession()
{

}

void PacketSession::Start()
{
	// Recv ½ÃÀÛ
	RecvPacket();
}

void PacketSession::RecvPacket()
{
	auto buffer = std::make_shared<std::vector<char>>(1024);
	auto self(shared_from_this());

	GetSocket().async_read_some(boost::asio::buffer(*buffer), [this, buffer](const boost::system::error_code& errorcode, std::size_t transferredbytes) {
		if (!errorcode)
		{
			std::string packet(buffer->data(), transferredbytes);
			HandlePacket(packet);
			RecvPacket();
		}
		});
}
