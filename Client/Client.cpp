#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"

const int THREAD_COUNT = 10;      // 총 스레드 수
const int SOCKETS_PER_THREAD = 100; // 스레드당 소켓 개수
const std::string SERVER_HOST = "127.0.0.1";
const short SERVER_PORT = 7777;

using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

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
        LOGI << "Connected Server!";
    }

    void OnDisconnected()
    {
        LOGI << "Disconnected Server!";
    }

    void SendPacket(const std::string& message)
    {
        Packet packet;
        std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
        std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
        packet.header.type = PacketType::YJ;
        packet.header.size = static_cast<uint32>(sizeof(PacketBuffer) + message.size());
        std::memcpy(packet.payload, message.c_str(), message.size());
        packet.tail.value = 255;

        Send(packet);
    }
};

void WorkerThread(boost::asio::io_context& ioContext, int socketCount)
{
    std::vector<std::shared_ptr<ServerSession>> sessions;

    std::string message(u8"Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server. Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.Hello Server.",128);
        for (int i = 0; i < socketCount; ++i)
        {
            // 세션 생성 및 소켓 관리
            auto session = std::make_shared<ServerSession>(ioContext, tcp::socket(ioContext));
            sessions.push_back(session);

            tcp::resolver resolver(ioContext);
            auto endpoints = resolver.resolve(SERVER_HOST, std::to_string(SERVER_PORT));

            auto self = session; // 세션의 수명을 보장하기 위해 self 참조 유지
            boost::asio::async_connect(session->GetSocket(), endpoints,
                [session, message](boost::system::error_code ec, tcp::endpoint)
                {
                    if (!ec)
                    {
                        session->Start();
                        LOGI << "Connection to Server!";

                        // 패킷 송신
                        session->SendPacket(message);
                    }
                    else
                    {
                        LOGE << "Connection Failed! Message : " << ec.message();
                    }
                });
        }

    ioContext.run();
}

int main()
{
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ClientLog.txt");

    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

    try
    {
        boost::asio::io_context ioContext;
        work_guard_type work_guard(ioContext.get_executor());

        // 스레드 풀 생성
        std::vector<std::thread> threads;

        for (int i = 0; i < THREAD_COUNT; ++i)
        {
            threads.emplace_back([&ioContext]() {
                WorkerThread(ioContext, SOCKETS_PER_THREAD);
                });
        }

        ioContext.run();

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
