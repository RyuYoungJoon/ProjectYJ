#include "pch.h"
#include "AsioServer.h"


int main()
{
	boost::asio::io_context ioContext;
	AsioServer server(ioContext, 0);
	
	//ioContext.run();
}
