#include "pch.h"
#include "ChatWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include <CommCtrl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

// Ŭ���� �̸�
#define WIN_CLASS_NAME L"ChatClientWindow"

// ��Ʈ�� ID ����
#define IDC_EDIT_MESSAGE 1001
#define IDC_BUTTON_SEND 1002
#define IDC_LIST_CHAT 1003
#define IDC_STATUS_BAR 1004


HWND g_hWnd = NULL; // ���� ������ ���
HWND g_hEditMessage = NULL; // �޽��� �Է� ���� ��Ʈ��
HWND g_hSendButton = NULL; // ���� ��ư
HWND g_hListChat = NULL; // ä�� ��� ����Ʈ �ڽ�
HWND g_hStatusBar = NULL; // ���¹�

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
	// ��Ʈ�� �ʱ�ȭ.
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	// ������ Ŭ����
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
		MessageBoxW(NULL, L"������ Ŭ���� ��� ����", L"ERROR", MB_ICONERROR);
		return;
	}

	// ������ ����
	g_hWnd = CreateWindowW(L"STATIC", L"Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, 600, 500, nullptr, nullptr, hInstance, nullptr);

	// �̰� �ǳ�?
	ClientSession::SetMainWin(g_hWnd);

	// ��Ʈ�� ����
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
