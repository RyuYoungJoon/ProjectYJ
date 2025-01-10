#pragma once
class AsioIoContext
{
public:
	AsioIoContext(boost::asio::io_context& ioContext);
	~AsioIoContext();

public:
	bool Init();
	bool Run();
	bool Stop();

	boost::asio::io_context& GetIoContext() { return m_ioContext; }

private:
	boost::asio::io_context& m_ioContext;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type>* m_work;
	std::vector<std::thread*> m_asioThread;
};
