#include "pch.h"
#include "AsioService.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include "ClientManager.h"
#include <filesystem>
#include "ObjectPool.h"
#include "ChatWindow.h"
#include "LoginWindow.h"
#include "LobbyWindow.h"
#include "PacketHandler.h"
#include "WinUtils.h"

#include <..\include\INIReader\ini.h>
#include <..\include\INIReader\ini.c>
#include <..\include\INIReader\INIReader.h>
#include <..\include\INIReader\INIReader.cpp>


string serverPort;
string serverIP;
int32 threadCnt;
int32 maxSessionCnt;

ClientServicePtr clientService;

// 윈도우 객체들
std::unique_ptr<LoginWindow> g_loginWindow;
std::unique_ptr<LobbyWindow> g_lobbyWindow;
std::unique_ptr<ChatWindow> g_chatWindow;

std::vector<std::thread> ConnectThreads;
std::vector<std::thread> ClientThreads;
using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

HWND g_hMainWnd = NULL;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// 메인 윈도우 등록 및 생성
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"MainWindowClass";
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&wcex))
	{
		MessageBoxW(NULL, L"메인 윈도우 클래스 등록 실패", L"오류", MB_ICONERROR);
		return;
	}

	g_hMainWnd = CreateWindowW(L"MainWindowClass", L"채팅 클라이언트", WS_OVERLAPPEDWINDOW,
		0, 0, 1, 1, nullptr, nullptr, hInstance, nullptr);

	if (!g_hMainWnd)
	{
		MessageBoxW(NULL, L"메인 윈도우 생성 실패", L"오류", MB_ICONERROR);
		return;
	}

	// 메인 윈도우는 보이지 않게 함 (메시지 처리용)
	ShowWindow(g_hMainWnd, SW_HIDE);
	UpdateWindow(g_hMainWnd);

	// 로그인, 채팅방 목록, 채팅창 초기화
	g_loginWindow = std::make_unique<LoginWindow>();
	g_lobbyWindow = std::make_unique<LobbyWindow>();
	g_chatWindow = std::make_unique<ChatWindow>();

	g_loginWindow->SetMainWnd(g_hMainWnd);
	g_lobbyWindow->SetMainWnd(g_hMainWnd);
	g_chatWindow->SetMainWnd(g_hMainWnd);

	if (!g_loginWindow->Init(hInstance) ||
		!g_lobbyWindow->Init(hInstance) ||
		!g_chatWindow->Init(hInstance))
	{
		MessageBoxW(NULL, L"윈도우 초기화 실패", L"오류", MB_ICONERROR);
		return;
	}

	// 로그인 창부터 표시
	g_loginWindow->Show();
}

void RunChatWindow()
{
	MSG msg = { 0 };
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
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
	threadCnt = reader.GetInteger("client", "threadCnt", 10);
	maxSessionCnt = reader.GetInteger("client", "maxSessionCnt", 10);
	long packetPoolSize = reader.GetInteger("client", "PacketPoolSize", 10000);

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

		// 패킷 라우터 Init
		PacketRouter::GetInstance().Init(0, []()-> std::shared_ptr<PacketProcessor>
			{
				return std::make_shared<PacketHandler>();
			});

		// 패킷 풀 Init
		PacketPool::GetInstance().Init(packetPoolSize);

		clientService = std::make_shared<AsioClientService>(
			ioContext,
			serverIP,
			serverPort,
			[](boost::asio::io_context* ioContext, tcp::socket* socket) -> std::shared_ptr<AsioSession> {
				return make_shared<ClientSession>(ioContext, socket);
			},
			maxSessionCnt);

		// 채팅 UI 초기화
		InitWindow(hInstance, nCmdShow);

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

					//ClientManager::GetInstance().Process();
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

		RunChatWindow();

		for (auto& t : ConnectThreads) {
			if (t.joinable()) {  // join 가능한지 확인 후 호출 (이미 join()된 스레드에 다시 join()하면 오류 발생)
				t.join();
			}
		}

		PacketPool::GetInstance().Clean();

		work_guard.reset();
		ioContext->stop();

		for (auto& t : asioThread)
		{
			if (t.joinable())
			{
				t.join();
			}
		}

		delete ioContext;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}

LRESULT MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_LOGIN_SUCCESS:
		// 로그인 성공 시 채팅방 목록 창 표시
		if (g_loginWindow && g_lobbyWindow)
		{
			g_lobbyWindow->Show(g_loginWindow->GetUserID());
		}
		return 0;

	case WM_ENTER_CHATROOM:
		// 채팅방 입장 시 채팅창 표시
		if (g_chatWindow)
		{
			ChatRoomResponseData* data = (ChatRoomResponseData*)lParam;
			if (data)
			{
				// 채팅창 제목 설정
				g_chatWindow->SetTitle(WinUtils::StringToWString("채팅방: " + data->roomName));

				// 채팅창 표시
				g_chatWindow->Show();

				// 채팅방 입장 메시지 추가
				g_chatWindow->AddChatMessage(L"채팅방 '" +
					WinUtils::StringToWString(data->roomName) +
					L"'에 입장했습니다.");

				// 메모리 해제
				delete data;
			}
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
