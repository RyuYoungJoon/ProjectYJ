#pragma once

using boost::asio::ip::tcp;

class AsioSession
{
public:
	virtual ~AsioSession() = default;

	// ���� ����.
	virtual void start() = 0;
	void Send(const std::string& message); // �ϴ� string
	void Recv();

private:
	tcp::socket m_socket;
};

