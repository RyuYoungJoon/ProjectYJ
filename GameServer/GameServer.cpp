#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"

int main()
{
	try
	{
		boost::asio::io_context IoContext;

		auto sessionMaker = [&IoContext]() -> std::shared_ptr<AsioSession>
			{
				return std::make_shared<AsioSession>(IoContext, tcp::socket(IoContext));
			};

		short port = 12345;
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

		IoContext.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}
}
