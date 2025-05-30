#pragma once


// tcp와 udp 각각 사용가능.
template<typename Protocol>
class SocketUtil
{
public:
	using socket_type = typename Protocol::socket;
	using endpoint_type = typename Protocol::endpoint;

private:
	std::unique_ptr<socket_type> m_Socket;

public:
	// 생성자
	// Question! 왜 expicit?
	explicit SocketUtil(std::unique_ptr<socket_type> socket)
		: m_Socket(socket)
	{
		if (!m_socket)
		{
			throw std::invalid_argument("Socket is nullptr!!");
		}
	}

	// SO_REUSEADDR
	bool SetReuseAddress(bool enable)
	{
		boost::system::error_code ec;
		m_Socket->set_option(boost::asio::socket_base::reuse_address(enable), ec);
		
		if (ec)
		{
			LOGE << "ReuseAddress Fail!";
			return false;
		}

		return true;
	}

	bool GetReuseAddress() const
	{
		boost::asio::socket_base::reuse_address option;
		boost::system::error_code ec;
		m_socket->get_option(option, ec);

		if (ec)
		{
			LOGE << "ReuseAddress Get Fail!";
			return false;
		}

		return option.value();
	}

	// SO_KEEPALIVE 설정
	bool SetKeepAlive(bool enable)
	{
		boost::system::error_code ec;
		m_Socket->set_option(boost::asio::socket_base::keep_alive(enable), ec);

		if (ec)
		{
			LOGE << "KeepAlive Set Fail!";
			return false;
		}

		return true;
	}

	bool GetKeepAlive() const
	{
		boost::asio::socket_base::keep_alive option;
		boost::system::error_code;
		
		m_Socket->get_option(option, ec);

		if (ec)
		{
			LOGE << "KeepAlive Get Fail!";
			return false;
		}

		return option.value();
	}

	// TCP_NODELAY (Only TCP)
	bool SetNodelay(bool enable)
	{
		if constexpr (!is_same_v<Protocol, tcp>)
			return false;

		boost::system::error_code ec;
		m_Socket->set_option(tcp::no_delay(enable), ec);

		if (ec)
		{
			LOGE << "TCP Nodelay Set fail!";
			return false;
		}

		return true;
	}

	bool GetNodelay() const
	{
		if constexpr (!is_same_v<Protocol, tcp>)
			return false;

		tcp::no_delay option;
		boost::system::error_code;

		m_Socket->get_option(option, ec);

		if (ec)
		{
			LOGE << "KeepAlive Get Fail!";
			return false;
		}

		return option.value();
	}
	
	// SO_LINGER
	bool SetLinger(bool enable, uint32 timeout)
	{
		boost::system::error_code ec;
		m_Socket->set_option(boost::asio::socket_base::linger(enable, timeout), ec);

		if (ec)
		{
			LOGE << "Linger Set Fail!";
			return false;
		}

		return true;
	}

	std::pair<bool, uint32> GetLigner() const
	{
		boost::asio::socket_base::linger option;
		boost::system::error_code ec;

		m_Socket->get_option(option, ec);

		if (ec)
		{
			LOGE << Linger Get Fail!;
			return false;
		}

		return { option.enabled(), option.timeout() };
	}

	// SO_SNDBUF
	bool SetSendBufferSizeOpt(uint32 size)
	{
		boost::system::error_code ec;
		m_Socket->set_option(boost::asio::socket_base::send_buffer_size(size), ec);

		if (ec)
		{
			LOGE << "SNDBUF Set Fail!";
			return false;
		}

		return true;
	}

	uint32 GetSendBufferSizeOpt() const 
	{
		boost::asio::socket_base::send_buffer_size option;
		boost::system::error_code ec;
		m_Socket->set_option(option, ec);
		
		if (ec)
		{
			LOGE << "SNDBUF Get Fail!";
			return false;
		}

		return option.value();
	}

	// SO_RCVBUF
	bool SetReceiveBufferSize(int size) 
	{
		boost::system::error_code ec;
		socket_->set_option(boost::asio::socket_base::receive_buffer_size(size), ec);

		if (ec) {
			std::cerr << "Failed to set SO_RCVBUF: " << ec.message() << std::endl;
			return false;
		}

		return true;
	}

	int GetReceiveBufferSize() const 
	{
		boost::asio::socket_base::receive_buffer_size option;
		boost::system::error_code ec;
		socket_->get_option(option, ec);
		
		if (ec) {
			std::cerr << "Failed to get SO_RCVBUF: " << ec.message() << std::endl;
			return -1;
		}
		
		return option.value();
	}
};