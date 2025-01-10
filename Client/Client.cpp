#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"

int main()
{
    try
    {
        boost::asio::io_context ioContext;

        auto sessionMaker = [&ioContext]() -> std::shared_ptr<AsioSession>
            {
                return std::make_shared<AsioSession>(ioContext, tcp::socket(ioContext));
            };

        std::string host = "127.0.0.1";
        short port = 27931;

        auto clientService = std::make_shared<AsioClientService>(ioContext, host, port, sessionMaker);


        if (clientService->Start())
        {
            std::cout << "Client is trying to connect to " << host << ":" << port << std::endl;
        }
        else
        {
            std::cerr << "Failed to start the client." << std::endl;
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
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
