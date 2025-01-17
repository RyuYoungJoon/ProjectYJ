#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"

const int THREAD_COUNT = 10;      // 총 스레드 수
const int SOCKETS_PER_THREAD = 10; // 스레드당 소켓 개수
const std::string SERVER_HOST = "127.0.0.1";
const short SERVER_PORT = 27931;

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
        packet.header.type = PacketType::YJ;
        packet.header.size = sizeof(PacketBuffer) + message.size();
        std::memcpy(packet.payload, message.c_str(), message.size());
        packet.tail.value = 255;

        Send(packet);
    }
};

void WorkerThread(boost::asio::io_context& ioContext, int socketCount)
{
    std::vector<std::shared_ptr<ServerSession>> sessions;

    for (int i = 0; i < socketCount; ++i)
    {
        // 세션 생성 및 소켓 관리
        auto session = std::make_shared<ServerSession>(ioContext, tcp::socket(ioContext));
        sessions.push_back(session);

        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(SERVER_HOST, std::to_string(SERVER_PORT));

        auto self = session; // 세션의 수명을 보장하기 위해 self 참조 유지
        boost::asio::async_connect(session->GetSocket(), endpoints,
            [session](boost::system::error_code ec, tcp::endpoint)
            {
                if (!ec)
                {
                    session->Start();
                    std::cout << "[Info] Connected to server!" << std::endl;

                    // 패킷 송신
                    session->SendPacket("Hello Server from client socket.");
                }
                else
                {
                    std::cerr << "[Error] Connection failed: " << ec.message() << std::endl;
                }
            });
    }

    ioContext.run();
}

int main()
{
    try
    {
        //boost::asio::io_context IoContext;
      
        //std::string host = "127.0.0.1";
        //short port = 27931;

        //auto serverSession = [](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<ServerSession>
        //    {
        //        return std::make_shared<ServerSession>(ioContext, std::move(socket));
        //    };

        //auto clientService = std::make_shared<AsioClientService>(
        //    IoContext,
        //    host, 
        //    port, 
        //    serverSession);

        //if (clientService->Start())
        //{
        //    std::cout << "Client is trying to connect to " << host << ":" << port << std::endl;
        //}
        //else
        //{
        //    std::cerr << "Failed to start the client." << std::endl;
        //    return -1;
        //}

        //while (true)
        //{
        //    std::string message(u8"Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.");

        //    cout << "[Client] Send Packet : " << message << "size : " << sizeof(message) << endl;
        //    serverSession->SendPacket(message); // 패킷 송신
        //    this_thread::sleep_for(100ms);
        //}


        //std::vector<std::thread> m_asioThread;
        //for (int i = 0; i < 4; ++i)
        //{
        //    m_asioThread.emplace_back([&IoContext]() {
        //        IoContext.run();
        //        });
        //}

        //for (auto& thread : m_asioThread)
        //{
        //    if (thread.joinable())
        //        thread.join();
        //}

        boost::asio::io_context ioContext;

        // 스레드 풀 생성
        std::vector<std::thread> threads;

        for (int i = 0; i < THREAD_COUNT; ++i)
        {
            threads.emplace_back([&ioContext]() {
                WorkerThread(ioContext, SOCKETS_PER_THREAD);
                });
        }

        // 모든 스레드 종료 대기
        for (auto& thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
