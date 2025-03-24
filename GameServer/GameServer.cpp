#include "pch.h"
#include "AsioService.h"
#include "AsioAcceptor.h"
#include "AsioSession.h"
#include "GameSession.h"
#include "TaskQueue.h"
#include "ServerAnalyzer.h"
#include "PacketRouter.h"
#include "ObjectPool.h"

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>


std::atomic<bool> g_IsRunning(true);
shared_ptr<AsioServerService> serverService = nullptr;

void InputThread(boost::asio::io_context* ioContext)
{
	int16 input;
	while (g_IsRunning)
	{
		//std::getline(std::cin, input);
		cin >> input;
		switch (input)
		{
		case 0:
			g_IsRunning = false;
			LOGI << "Server Release";
			serverService->CloseService();

			PacketQueue::GetInstance().Shutdown();
			LOGI << "Task queue shutdown complete";

			PacketPool::GetInstance().Clean();
			LOGI << "PacketPool Clean complete";

			ioContext->stop();

			break;
		case 1:
			LOGD << "TotalPacketRecvCount : " << ServerAnalyzer::GetInstance().GetTotalRecvCount();
			break;
		default:
			break;
		}
	}
}

int main()
{
	// TODO : 로그 설정하기, Config 설정하기 함수로 나누기.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(106292);

	char filePath[MAX_PATH] = { 0 };
	::GetModuleFileNameA(nullptr, filePath, MAX_PATH);
	::PathRemoveFileSpecA(filePath);

	string iniPath = "\\ServerConfig.ini";
	string ConfigPath = filePath + iniPath;

	if (!std::filesystem::exists(ConfigPath))
	{
		LOGE << "File Not found" << ConfigPath;
	}

	INIReader reader(ConfigPath);
	if (reader.ParseError() < 0)
	{
		LOGE << "Can't load config";
	}

	string logPath = filePath;
	logPath.append("\\log\\");

	if (::GetFileAttributesA(logPath.c_str()) == -1)
	{
		::CreateDirectoryA(logPath.c_str(), nullptr);
	}

	char strInfoPathTemp[MAX_PATH] = { 0 };
	sprintf_s(strInfoPathTemp, sizeof(strInfoPathTemp), "%sserver.log", logPath.c_str());

	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(strInfoPathTemp);
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);
	
	try
	{
		boost::asio::io_context* IoContext = new boost::asio::io_context();
		
		string serverPort = reader.Get("server", "Port", "7777");
		string serverIP = reader.Get("server", "Address", "127.0.0.1");
		long threadCnt = reader.GetInteger("server", "ThreadCnt", 4);
		long packetPoolSize = reader.GetInteger("server", "PacketPoolSize", 10000);
		long sessionPoolSize = reader.GetInteger("server", "SessionPoolSize", 10000);

		// 패킷 라우터 Init
		//PacketHandler* handler = &PacketHandler::GetInstance();

		PacketRouter::GetInstance().Init(0, []()-> std::shared_ptr<PacketProcessor>
			{
				return std::make_shared<PacketHandler>();
			});

		// 패킷 풀 Init
		PacketPool::GetInstance().Init(packetPoolSize);
		LOGI << "PacketPool initialized with " << PacketPool::GetInstance().GetTotalCount() << "size";

		// 세션 풀 Init
		auto gameSessionPool = std::make_shared<ObjectPool<GameSession>>();
		gameSessionPool->InitPool(10000);

		serverService = std::make_shared<AsioServerService>(
			IoContext, 
			serverIP,
			serverPort,
			[gameSessionPool](boost::asio::io_context* ioContext ,tcp::socket* socket) -> std::shared_ptr<AsioSession> {
				shared_ptr<GameSession> session = gameSessionPool->Pop();
				session->InitSession(ioContext, socket);
				return session;
			});

		if (serverService->Start())
		{
			LOGI << "[SERVER INFO] Server is running and waiting for connections on IP : " << serverIP << ", Port : " << serverPort;
		}
		else
		{
			LOGE << "Failed to Start the Server";
			return -1;
		}

		// TODO : 실제로 쓰는것만 놔두기 OR TaskQueue 추가 개발하기
		std::thread ioThread(InputThread, std::ref(IoContext));
		PacketQueue::GetInstance().Initialize();
		LOGI << "Task queue initialized with " << std::thread::hardware_concurrency() / 2 << " worker threads";
	
		std::vector<std::thread> m_asioThread;
		for (int i = 0; i < threadCnt; ++i)
		{
			m_asioThread.emplace_back([IoContext]() {
				IoContext->run();
				});
		}

		for (auto& thread : m_asioThread)
		{
			if (thread.joinable())
				thread.join();
		}

		//plog::get()->flush();
		

		ioThread.join();

		IoContext->stop();
		delete IoContext;
	}
	catch (const std::exception& e)
	{
		LOGE << "Exception : " << e.what();
	}
}
