#pragma once

using boost::asio::ip::tcp;

class AsioSession
{
public:
	virtual ~AsioSession() = default;

	// 연결 시작.
	virtual void start() = 0;
	void Send(const std::string& message); // 일단 string
	void Recv();

private:
	tcp::socket m_socket;
};

