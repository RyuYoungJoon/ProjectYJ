#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"

class ServerSession : public AsioSession
{
public:
    ServerSession(boost::asio::io_context& iocontext, tcp::socket socket)
        : AsioSession(iocontext, std::move(socket))
    {

    }

    void OnSend(int32 len)
    {
        cout << "OnSend 호출" << endl;
    }

    int32 OnRecv(BYTE* buffer, int32 len)
    {
        return len;
    }

    void OnConnected()
    {
        cout << "[Info] Connected Server!" << endl;
    }

    void OnDisconnected()
    {
        cout << "[Info] Disconnected Server!" << endl;
    }
};

int main()
{
    try
    {
        boost::asio::io_context IoContext;

        auto sessionMaker = [&IoContext]() -> std::shared_ptr<AsioSession>
            {
                return std::make_shared<AsioSession>(IoContext, tcp::socket(IoContext));
            };

        std::string host = "127.0.0.1";
        short port = 27931;

        auto clientService = std::make_shared<AsioClientService>(
            IoContext,
            host, 
            port, 
            [](boost::asio::io_context& iocontext, tcp::socket socket) -> std::shared_ptr<AsioSession> 
            {
                return std::make_shared<ServerSession>(iocontext, std::move(socket));
            });


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
