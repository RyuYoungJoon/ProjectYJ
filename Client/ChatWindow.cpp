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
LPCTSTR lpszClass = L"Window Class Name";

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
	wcex.lpszClassName = lpszClass;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&wcex))
	{
		MessageBoxW(NULL, L"������ Ŭ���� ��� ����", L"ERROR", MB_ICONERROR);
		return;
	}

	// ������ ����
	g_hWnd = CreateWindowW(lpszClass, L"Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

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
    switch (uMessage){
    case WM_CREATE:
		break;

    case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_BUTTON_SEND) {
			//SendChatMessage();
			break;
		}
	
	}
	break;
    case WM_CLIENT_CONNECTED: // �����
	{
		//OnClientConnect();
	
	}
	break;
    case WM_CLIENT_DISCONNECT: // ���� ������
	{
       // OnClientDisconnect();
		

	}
	break;
    case WM_CLIENT_RECV: // �޽��� ����
    {
		ChatMessageData* data = (ChatMessageData*)lParam;
        if (data) {
            //OnMessageRecv(data->sender, data->message);
            delete data; // �޸� ����
        }
    }
	break;
    case WM_SIZE:
    {
        // ��Ʈ�� ũ�� ����
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        // ����Ʈ�ڽ� ũ�� ����
        SetWindowPos(g_hListChat, NULL,
            10, 10,
            rcClient.right - 20, rcClient.bottom - 90,
            SWP_NOZORDER);

        // �Է�â ũ�� ����
        SetWindowPos(g_hEditMessage, NULL,
            10, rcClient.bottom - 70,
            rcClient.right - 100, 40,
            SWP_NOZORDER);

        // ���� ��ư ũ�� ����
        SetWindowPos(g_hSendButton, NULL,
            rcClient.right - 80, rcClient.bottom - 70,
            70, 40,
            SWP_NOZORDER);

        // ���¹� ũ�� ����
        SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMessage, wParam, lParam);
}

void CreateChatControl(HWND hwnd)
{
	HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"���� ���");

	// ä�� ����Ʈ�ڽ�
	g_hListChat = CreateWindowW(L"LISTBOX", NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
		10, 10, 560, 380, hwnd, (HMENU)IDC_LIST_CHAT, NULL, NULL);
	SendMessage(g_hListChat, WM_SETFONT, (WPARAM)hFont, TRUE);

	// �޽��� �Է�â
	g_hEditMessage = CreateWindowW(L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		10, 300, 300, 10, hwnd, (HMENU)IDC_EDIT_MESSAGE, NULL, NULL);
	SendMessage(g_hEditMessage, WM_SETFONT, (WPARAM)hFont, TRUE);

	// ���� ��ư
	g_hSendButton = CreateWindowW(L"BUTTON", L"����",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		500, 400, 70, 40, hwnd, (HMENU)IDC_BUTTON_SEND, NULL, NULL);
	SendMessage(g_hSendButton, WM_SETFONT, (WPARAM)hFont, TRUE);

	// ���¹�
	g_hStatusBar = CreateWindowW(STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS_BAR, NULL, NULL);

	// ���� ���� ���� ǥ��
	//std::wstring statusText = L"����: " + StringToWString(serverIP) + L":" + StringToWString(serverPort) + L" (���� ��...)";
	//SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());
}
