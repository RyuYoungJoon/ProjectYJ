#pragma once

class AsioNetwork
{
public:
	
	AsioNetwork() : m_IoContext(), m_Work(boost::asio::make_work_guard(m_IoContext)) {
		std::cout << "AsioNetwork Init!" << std::endl;
	}

	void DoServerStart(uint16 port);
	void DoClientStart(const std::string& host, uint16 port);
	void Run();
	void Stop();


private:
	boost::asio::io_context m_IoContext;	// io ��ü
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_Work; // m_IoContext�� ��� ���� ���·� ����
	std::shared_ptr<AsioServer> m_Server;
};

