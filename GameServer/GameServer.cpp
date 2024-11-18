#include "pch.h"
#include "AsioServer.h"
#include "AsioIoContext.h"

int main()
{
	AsioIoContext ioContext;
	ioContext.Init();

	AsioServer server(*ioContext.GetIoContext(), 12345);

	ioContext.Run();
}
