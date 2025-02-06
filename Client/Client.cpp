#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"
#include <filesystem>

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution<int> dist(10, 100);

string serverPort;
string serverIP;
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

	int OnConnected()
	{
		int random = dist(dre);

		for (int i = 0; i < random; ++i)
		{
			SendPacket("hihihi");
			// 시도 횟수
			sendCnt.fetch_add(1);
		}

		// TODO : 아예 카운트 찍는 기능을 만들자.
		// sendCnt == random
		// 내가 시도한 횟수 SendCnt
		// 내가 시도한 전체 횟수 TotalTryCnt
		LOGI << "SendCnt [" << sendCnt << "]";
		
		return random;
	}

	void OnDisconnected()
	{
		LOGI << "Disconnected Server!";

		//std::this_thread::sleep_for(100ms);
		Connect(serverIP, serverPort);
		
		LOGI << "TryConnected Server!";
	}

	void SendPacket(const std::string& message)
	{
		Packet packet;
		std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
		std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
		packet.header.type = PacketType::YJ;
		packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
		std::memcpy(packet.payload, message.c_str(), message.size());
		packet.tail.value = 255;

		Send(packet);
	}
private:
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

	serverPort = reader.Get("client", "Port", "7777");
	serverIP = reader.Get("client", "Address", "127.0.0.1");
	int32 threadCnt = reader.GetInteger("client", "ThreadCnt", 10);
	int32 maxSessionCnt = reader.GetInteger("client", "MaxSessionCount", 100);

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
			maxSessionCnt);

		for (int i = 0; i < threadCnt; ++i)
		{
			threads.emplace_back([]() {
				if (clientService->Start())
				{
					LOGI << "[SERVER INFO] Server is running and waiting for connections on port " << serverPort;
				}
				else
				{
					LOGE << "Failed to Start the Server";
					return -1;
				}
			});
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
