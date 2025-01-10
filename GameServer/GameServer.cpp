#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
#include "AsioIoContext.h"
#include "Protocol.h"

int main()
{
	try
	{
		boost::asio::io_context IoContext;
		
        auto sessionMaker = [&IoContext]() -> std::shared_ptr<AsioSession>
			{
			    return std::make_shared<AsioSession>(IoContext, tcp::socket(IoContext));
			};
		
		short port = 27931;
		auto serverService = std::make_shared<AsioServerService>(IoContext, port, sessionMaker);
		 
		if (serverService->Start())
		{
			std::cout << "Server is running and waiting for connections on port " << port << std::endl;
		}
		else
		{
			std::cerr << "Failed to start the server." << std::endl;
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
