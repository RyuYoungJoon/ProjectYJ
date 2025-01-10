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
		AsioIoContext ioContext(IoContext);

		ioContext.Init();

        auto sessionMaker = [ioContext](boost::asio::io_context& IoContext) -> std::shared_ptr<AsioSession>
			{
			    return std::make_shared<AsioSession>(IoContext, tcp::socket(IoContext));
			};
		

		short port = 27931;
		auto serverService = std::make_shared<AsioServerService>(ioContext.GetIoContext(), port, sessionMaker);
		 
		if (serverService->Start())
		{
			std::cout << "Server is running and waiting for connections on port " << port << std::endl;
		}
		else
		{
			std::cerr << "Failed to start the server." << std::endl;
			return -1;
		}

		ioContext.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}
}
