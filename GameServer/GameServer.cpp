#include "pch.h"
#include "AsioServer.h"
#include "AsioIoContext.h"

int main()
{
	try
	{
		boost::asio::io_context IoContext;

		AsioServer MyServer(IoContext, 12345);

		IoContext.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}
}
