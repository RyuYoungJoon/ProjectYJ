﻿#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
#include "GameSession.h"
#include "TaskQueue.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ServerLog.txt");
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	try
	{
		boost::asio::io_context IoContext;

		// Temporary Test Port
		short port = 7777;
		auto serverService = std::make_shared<AsioServerService>(
			IoContext, 
			port, 
			[](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
				return std::make_shared<GameSession>(ioContext, std::move(socket));
			});

		if (serverService->Start())
		{
			LOGI << "[SERVER INFO] Server is running and waiting for connections on port " << endl;
		}
		else
		{
			LOGE << "Failed to Start the Server";
			return -1;
		}
		
		std::thread WorkerThread([]() {
			TaskQueue::GetInstance().ProcessTask();
			});


		std::vector<std::thread> m_asioThread;
		for (int i = 0; i < 4; ++i)
		{
			m_asioThread.emplace_back([&IoContext]() {
				IoContext.run();
				});
		}

		WorkerThread.detach();

		for (auto& thread : m_asioThread)
		{
			if (thread.joinable())
				thread.join();
		}

	}
	catch (const std::exception& e)
	{
		LOGE << "Exception : " << e.what();
	}
}
