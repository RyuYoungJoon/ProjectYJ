#include "pch.h"
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
ClientServicePtr clientService;

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
		boost::asio::io_context* ioContext = new boost::asio::io_context();
		work_guard_type work_guard(ioContext->get_executor());
		clientService = std::make_shared<AsioClientService>(
			ioContext,
			serverIP,
			serverPort,
			[](boost::asio::io_context* ioContext, tcp::socket socket) -> std::shared_ptr<AsioSession> {
				return std::make_shared<ClientSession>(ioContext, std::move(socket));
			},
			maxSessionCnt);
		//// 스레드 생성
		for (int i = 0; i < threadCnt; ++i)
		{
			ConnectThreads.emplace_back([ioContext]() {
				try {
					if (clientService->Start())
					{
						LOGI << "[SERVER INFO] Server is running and waiting for connections on port " << serverPort;
					}
					else
					{
						LOGE << "Failed to Start the Server";
						return -1;
					}

					ClientManager::GetInstance().Process();
				}
				catch (const std::exception& e) {
					LOGE << "Thread exception: " << e.what();
					return -2;
				}
				catch (...) {
					LOGE << "Unknown error occurred in thread!";
					return -3;
				}
				
			});
		}

		std::vector<std::thread> asioThread;
		for (int i = 0; i < threadCnt; ++i) 
		{
			asioThread.emplace_back([&ioContext]() {
				ioContext->run();
				});
		}

		for (auto& t : ConnectThreads) {
			if (t.joinable()) {  // join 가능한지 확인 후 호출 (이미 join()된 스레드에 다시 join()하면 오류 발생)
				t.join();
			}
		}

		for (auto& t : asioThread)
		{
			if (t.joinable())
			{
				t.join();
			}
		}

		ioContext->stop();

		delete ioContext;

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
