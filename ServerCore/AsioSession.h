#pragma once

using boost::asio::ip::tcp;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
public:
	AsioSession(tcp::socket socket)
		: m_socket(std::move(socket)) {}

	virtual ~AsioSession();

	// 연결 시작.
	void Start();
	void Send(const std::string& message); // 일단 string
	void DoRead();
	void DoWrite();

protected:
	tcp::socket& GetSocket() { return m_socket; }

private:
	tcp::socket m_socket;

	// TODO : string 대신 패킷버퍼로 변경 예정
	std::queue<string> writeBuffer;
	std::string readBuffer;
public:
};