﻿#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
#include "GameSession.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	try
	{
		boost::asio::io_context IoContext;

		// Temporary Test Port
		short port = 27931;
		auto serverService = std::make_shared<AsioServerService>(
			IoContext, 
			port, 
			[](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
				return std::make_shared<GameSession>(ioContext, std::move(socket));
			});

		if (serverService->Start())
		{
			std::cout << "[SERVER INFO] Server is running and waiting for connections on port " << port << std::endl;
		}
		else
		{
			std::cerr << "[ERROR] Failed to start the server." << std::endl;
			return -1;
		}

		// WorkerThread 생성



		IoContext.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}
}
