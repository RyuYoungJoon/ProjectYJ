#include "pch.h"
#include "ChatWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include <CommCtrl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

// 클래스 이름
#define WIN_CLASS_NAME L"ChatClientWindow"

// 컨트롤 ID 정의
#define IDC_EDIT_MESSAGE 1001
#define IDC_BUTTON_SEND 1002
#define IDC_LIST_CHAT 1003
#define IDC_STATUS_BAR 1004


HWND g_hWnd = NULL; // 메인 윈도우 헨들
HWND g_hEditMessage = NULL; // 메시지 입력 에딧 컨트롤
HWND g_hSendButton = NULL; // 전송 버튼
HWND g_hListChat = NULL; // 채팅 기록 리스트 박스
HWND g_hStatusBar = NULL; // 상태바

extern ClientServicePtr clientService;
extern string serverIP;
extern string serverPort;

std::mutex g_chatMutex;
std::vector<std::wstring> g_chatMessage;
bool g_isConnect = false;

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
void CreateChatControl(HWND hwnd);

void InitChatWindow(HINSTANCE hInstance, int nCmdShow)
{
	// 컨트롤 초기화.
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	// 윈도우 클래스
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WIN_CLASS_NAME;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&wcex))
	{
		MessageBoxW(NULL, L"윈도우 클래스 등록 에러", L"ERROR", MB_ICONERROR);
		return;
	}

	// 윈도우 생성
	g_hWnd = CreateWindowW(L"STATIC", L"Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, 600, 500, nullptr, nullptr, hInstance, nullptr);

	// 이게 되나?
	ClientSession::SetMainWin(g_hWnd);

	// 컨트롤 생성
	CreateChatControl(g_hWnd);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

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

LRESULT WinProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

void CreateChatControl(HWND hwnd)
{
}
