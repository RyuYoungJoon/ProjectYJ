#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"
#include <filesystem>

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>

const int THREAD_COUNT = 10;      // 총 스레드 수
const int SOCKETS_PER_THREAD = 100; // 스레드당 소켓 개수
//const std::string SERVER_HOST = "192.168.21.96";
const std::string SERVER_HOST = "127.0.0.1";
const short SERVER_PORT = 7777;
ClientServicePtr clientService;

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
		SendPacket("hihihi");
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
		packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + message.size() + sizeof(PacketTail));
		std::memcpy(packet.payload, message.c_str(), message.size());
		packet.tail.value = 255;

		Send(packet);
	}
};

int main()
{
	// 파일 경로 뽑기
	char filePath[MAX_PATH] = { 0 };
	string iniPath = "\\ClientConfig.ini";
	::GetModuleFileNameA(nullptr, filePath, MAX_PATH);
	::PathRemoveFileSpecA(filePath);

	// Config폴더 설정
	string ConfigPath = filePath + iniPath;
	if (!std::filesystem::exists(ConfigPath))
	{
		LOGE << "File Not found" << ConfigPath;
	}

	// Config 파일 읽기
	INIReader reader(ConfigPath);
	if (reader.ParseError() < 0)
	{
		LOGE << "Can't load config";
	}

	string serverPort = reader.Get("server", "port", "7777");
	string serverIP = reader.Get("server", "address", "127.0.0.1");

	// 로그 폴더 설정
	string logPath = filePath;
	logPath.append("\\log\\");

	// 해당 경로에 폴더가 없으면 생성해라.
	if (::GetFileAttributesA(logPath.c_str()) == -1)
	{
		::CreateDirectoryA(logPath.c_str(), nullptr);
	}

	char strInfoPathTemp[MAX_PATH] = { 0 };
	sprintf_s(strInfoPathTemp, sizeof(strInfoPathTemp), "%sclient.log", logPath.c_str());

	// plog 선언
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(strInfoPathTemp);
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	try
	{
		boost::asio::io_context ioContext;
		work_guard_type work_guard(ioContext.get_executor());

		//// 스레드 생성
		std::vector<std::thread> threads;

		clientService = std::make_shared<AsioClientService>(
			ioContext,
			serverIP,
			serverPort,
			[](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
				return std::make_shared<ServerSession>(ioContext, std::move(socket));
			},
			1000);

		if (clientService->Start())
		{
			LOGI << "[SERVER INFO] Server is running and waiting for connections on port " << serverPort;
		}
		else
		{
			LOGE << "Failed to Start the Server";
			return -1;
		}

		ioContext.run();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
