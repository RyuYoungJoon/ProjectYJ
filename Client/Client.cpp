#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"
#include <format>

const int THREAD_COUNT = 2;      // 총 스레드 수
const int SOCKETS_PER_THREAD = 10; // 스레드당 소켓 개수
const std::string SERVER_HOST = "127.0.0.1";
const short SERVER_PORT = 27931;

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
    std::list<std::shared_ptr<ServerSession>> sessions;

    std::string message(u8"Hello Server.");

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
                    std::cout << "[Info] Connected to server!" << std::endl;

                    // 패킷 송신
                    session->SendPacket(message);
                }
                else
                {
                    std::cerr << "[Error] Connection failed: " << ec.message() << std::endl;
                }
            });
        this_thread::sleep_for(100ms);
    }

    ioContext.run();
}

int main()
{
    try
    {
        boost::asio::io_context ioContext;
        work_guard_type work_guard(ioContext.get_executor());

        // 스레드 풀 생성
        std::vector<std::thread> threads;
        //std::array<std::thread, 10> threads;

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
