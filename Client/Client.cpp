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

    void SendPacket(const std::string& message)
    {
        Packet packet;
        std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
        std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
        packet.header.type = PacketType::defEchoString;
        packet.header.size = sizeof(PacketHeader) + message.size();
        std::memcpy(packet.payload, message.c_str(), message.size());

        Send(packet);
    }
};

int main()
{
    /*try
    {
        boost::asio::io_context IoContext;
      
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
    }*/

    boost::asio::io_context IoContext;

    tcp::resolver resolver(IoContext);
    auto endpoints = resolver.resolve("127.0.0.1", "27931");
    tcp::socket socket(IoContext);
    boost::asio::connect(socket, endpoints);

    auto session = std::make_shared<ServerSession>(IoContext, std::move(socket));
    
    session->Start(); // 응답 수신 시작

    while (true)
    {
        std::string message(u8"Hello Server.");
        cout << "[Client] Send Packet : " << message << endl;
        session->SendPacket(message); // 패킷 송신
        this_thread::sleep_for(1s);
    }

    IoContext.run();

    
    return 0;
}
