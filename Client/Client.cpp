﻿#include "pch.h"
#include "AsioService.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include "ClientManager.h"
#include <filesystem>

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>


string serverPort;
string serverIP;

std::vector<std::thread> ConnectThreads;
std::vector<std::thread> ClientThreads;
using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

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
		for (int i = 0; i < threadCnt; ++i)
		{
			ConnectThreads.emplace_back([&ioContext, &maxSessionCnt]() {

				ClientServicePtr clientService = std::make_shared<AsioClientService>(
					ioContext,
					serverIP,
					serverPort,
					[](boost::asio::io_context& ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
						return std::make_shared<ClientSession>(ioContext, std::move(socket));
					},
					maxSessionCnt);

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
				// 생각 정리
				// 세션이 전부 Connect 되면 Init을 해야하는가?
				// 세션 10개가 단체 행동 하는거 아닌가?
				// 세션이 들어오면 각자 Process 돌리고 나가면 되는거 아닌가?
				// 근데 그게 맞는 구조인가?
				AsioSessionPtr asioSession = nullptr;
				ClientManager::GetInstance().Init(asioSession);

				ClientManager::GetInstance().Process();

			});
		}

		for (auto& t : ConnectThreads) {
			if (t.joinable()) {  // join 가능한지 확인 후 호출 (이미 join()된 스레드에 다시 join()하면 오류 발생)
				t.join();
			}
		}

		//ioContext.stop();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
