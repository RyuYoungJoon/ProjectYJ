#include "pch.h"
#include "AsioService.h"
#include "AsioSession.h"
#include "ServerAnalyzer.h"
#include "ClientManager.h"
#include <filesystem>

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>


string serverPort;
string serverIP;
ClientServicePtr clientService;

std::vector<std::thread> ConnectThreads;
std::vector<std::thread> ClientThreads;
using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

class ServerSession : public AsioSession
{
public:
	ServerSession(boost::asio::io_context& iocontext, tcp::socket socket)
		: AsioSession(iocontext, std::move(socket))
	{
		std::random_device rd;
		std::default_random_engine dre(rd());
		std::uniform_int_distribution<int> dist(10, 100);
		targetRandomCnt = dist(dre);
	}

	void OnSend(int32 len)
	{
		packetCount++;

		if (packetCount >= targetRandomCnt)
		{
			//std::this_thread::sleep_for(1s);
			boost::asio::post(m_IoContext, [this]() {
				Disconnect();
				});
		}
	}

	int32 OnRecv(BYTE* buffer, int32 len)
	{
		return len;
	}

	void OnConnected()
	{
		// OnConnected에서는 connect 성공 로그만 찍자.
		//for (int i = 0; i < 100; ++i)
		//{
		//	SendPacket("hihisdvsdvsdvdsvhi");
		//}
		//
		//LOGD << "SendCount : " << ServerAnalyzer::GetInstance().GetSendCount() << ", TotalSendCount : " << ServerAnalyzer::GetInstance().GetTotalSendCount();

		//std::this_thread::sleep_for(3s);
		
		GetService()->AddSession(shared_from_this());

		GetService()->Process();

		//Disconnect();
	}

	void OnDisconnected()
	{
		std::this_thread::sleep_for(1s);
		LOGI << "Disconnected Server!";

		if (ServerAnalyzer::GetInstance().GetTotalSendCount() < 100000)
		{
			ServerAnalyzer::GetInstance().ResetSendCount();
			packetCount = 0;
			Connect(serverIP, serverPort);
		}
		
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
	int32 targetRandomCnt = 0;
	int32 packetCount = 0;
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
			ConnectThreads.emplace_back([&ioContext]() {
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
			});
		}
		
		ClientManager::GetInstance().Init();
		
		for (auto& t : ConnectThreads) {
			if (t.joinable()) {  // join 가능한지 확인 후 호출 (이미 join()된 스레드에 다시 join()하면 오류 발생)
				t.join();
			}
		}

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
