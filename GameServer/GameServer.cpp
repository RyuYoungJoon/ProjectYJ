#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
//#include "Logger.h"
#include "GameSession.h"


int main()
{
	static plog::ColorConsoleAppender<plog::TxtFormatter> cConsoleAppeder;
	plog::get()->addAppender(&cConsoleAppeder);
	//plog::init(plog::debug, &consoleAppender);

	PLOG_INFO << "서버 시작됨";
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	try
	{
		boost::asio::io_context IoContext;

		// Temporary Test Port
		short port = 77777;
		auto serverService = std::make_shared<AsioServerService>(
			IoContext, 
			port, 
			[](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
				return std::make_shared<GameSession>(ioContext, std::move(socket));
			});

		if (serverService->Start())
		{
			string message("[SERVER INFO] Server is running and waiting for connections on port ");
			cout <</* Logger::MyLog(message + to_string(port))*/"" << endl;
		}
		else
		{
			std::cerr << "[ERROR] Failed to start the server." << std::endl;
			return -1;
		}


		std::vector<std::thread> m_asioThread;
		for (int i = 0; i < 4; ++i)
		{
			m_asioThread.emplace_back([&IoContext]() {
				IoContext.run();
				});
		}

		for (auto& thread : m_asioThread)
		{
			if (thread.joinable())
				thread.join();
		}

	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}
}
