#pragma once

using boost::asio::ip::tcp;

class AsioSession
{
public:
	AsioSession() = default;

	//AsioSession(boost::asio::io_context& ioContext)
	//	: m_socket(ioContext) {}
	//
	virtual ~AsioSession();

	// 연결 시작.
	virtual void Start() = 0;
	void Send(const std::string& message); // 일단 string

protected:
	tcp::socket& GetSocket() { return m_socket; }

private:
	tcp::socket m_socket;

public:
	//AsioSession() = default;

};

class PacketSession : public AsioSession
{
public:
	PacketSession() = default;

	using AsioSession::AsioSession;

	/*PacketSession(boost::asio::io_context& ioContext)
		: AsioSession::AsioSession(ioContext) {}*/

	virtual ~PacketSession();

	void Start() override;

protected:
	// Recv
	void RecvPacket();

	// Handler
	virtual void HandlePacket(const std::string& packet) = 0;
};
