#pragma once

using boost::asio::ip::tcp;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
	AsioSession(tcp::socket socket)
		: m_socket(std::move(socket)) {}

	virtual ~AsioSession();

	// ���� ����.
	void Start();
	void Send(const std::string& message); // �ϴ� string
	void DoRead();
	void DoWrite();

protected:
	tcp::socket& GetSocket() { return m_socket; }

private:
	tcp::socket m_socket;

	// TODO : string ��� ��Ŷ���۷� ���� ����
	std::queue<string> writeBuffer;
	std::string readBuffer;
public:
};