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
LPCTSTR lpszClass = L"Window Class Name";

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
	wcex.lpszClassName = lpszClass;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassExW(&wcex))
	{
		MessageBoxW(NULL, L"윈도우 클래스 등록 에러", L"ERROR", MB_ICONERROR);
		return;
	}

	// 윈도우 생성
	g_hWnd = CreateWindowW(lpszClass, L"Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

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

void UpdateUI(bool isConnected)
{
	g_isConnect = isConnected;

	std::wstring statusText;
	if (isConnected) {
		statusText = L"서버: " + StringToWString(serverIP) + L":" + StringToWString(serverPort) + L" (연결됨)";
	}
	else {
		statusText = L"서버: " + StringToWString(serverIP) + L":" + StringToWString(serverPort) + L" (연결 끊김)";
	}

	SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());
	EnableWindow(g_hSendButton, isConnected);
}

void OnClientConnect()
{
	UpdateUI(true);
	AddChatMessage(L"서버에 연결되었습니다.");
}

void OnClientDisconnect()
{
	UpdateUI(false);
	AddChatMessage(L"서버와 연결이 끊어졌습니다.");
}

void OnMessageRecv(const std::string& sender, const std::string& message)
{
	std::wstring displayMessage = StringToWString(sender + ": " + message);
	AddChatMessage(displayMessage);
}

void SendChatMessage()
{
	if (!g_isConnect)
	{
		MessageBoxW(g_hWnd, L"서버에 연결되어있지 않습니다.", L"ERROR", MB_ICONERROR);
		return;
	}


	// 에딧컨트롤에서 텍스트 가져오기
	int length = GetWindowTextLengthW(g_hEditMessage);
	if (length == 0)
		return;

	std::vector<wchar_t> buffer(length + 1);
	GetWindowTextW(g_hEditMessage, buffer.data(), length + 1);
	std::wstring wMessage(buffer.data());
	std::string message = WStringToString(wMessage);

	// 메시지 전송
	auto& ClientManager = ClientManager::GetInstance();
	auto sessions = ClientManager.GetSessions();

	if (!sessions.empty())
	{
		auto session = sessions.begin()->second;
		if (session)
		{
			static_cast<ClientSession*>(session.get())->Send(message, PacketType::ChatReq);

			// 내가 보낸 메시지 표시.
			std::wstring myMessage = L"나 : " + wMessage;

			AddChatMessage(myMessage);

			// 입력창 clear
			SetWindowTextW(g_hEditMessage, L"");
		}
	}
	
}

void AddChatMessage(const std::wstring& message)
{
	// 채팅 메시지 추가 함수.
	{
		std::lock_guard<std::mutex> lock(g_chatMutex);
		g_chatMessage.push_back(message);

		while (g_chatMessage.size() > 100)
		{
			g_chatMessage.erase(g_chatMessage.begin());
		}
	}

	// 리스트 박스에 추가하기.
	SendMessage(g_hListChat, LB_ADDSTRING, 0, (LPARAM)message.c_str());

	// 자동 스크롤.
	int count = SendMessage(g_hListChat, LB_GETCOUNT, 0, 0);
	SendMessage(g_hListChat, LB_SETTOPINDEX, count - 1, 0);
}

std::wstring StringToWString(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	std::vector<wchar_t> buf(len);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf.data(), len);
	return std::wstring(buf.data());
}

std::string WStringToString(const std::wstring& wstr)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::vector<char> buf(len);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf.data(), len, NULL, NULL);
	return std::string(buf.data());
}

LRESULT WinProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage){
    case WM_CREATE:
		break;

    case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_BUTTON_SEND) {
			SendChatMessage();
			break;
		}
	}
	break;
	case WM_CLIENT_CONNECTED: // 연결됨
	{
		OnClientConnect();
	}
	break;
    case WM_CLIENT_DISCONNECT: // 연결 해제됨
	{
       OnClientDisconnect();
	}
	break;
    case WM_CLIENT_RECV: // 메시지 수신
    {
		ChatMessageData* data = (ChatMessageData*)lParam;
        if (data) {
            OnMessageRecv(data->sender, data->message);
            delete data; // 메모리 해제
        }
    }
	break;
    case WM_SIZE:
    {
        // 컨트롤 크기 조정
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        // 리스트박스 크기 조정
        SetWindowPos(g_hListChat, NULL,
            10, 10,
            rcClient.right - 20, rcClient.bottom - 90,
            SWP_NOZORDER);

        // 입력창 크기 조정
        SetWindowPos(g_hEditMessage, NULL,
            10, rcClient.bottom - 70,
            rcClient.right - 100, 40,
            SWP_NOZORDER);

        // 전송 버튼 크기 조정
        SetWindowPos(g_hSendButton, NULL,
            rcClient.right - 80, rcClient.bottom - 70,
            70, 40,
            SWP_NOZORDER);

        // 상태바 크기 조정
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
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");

	// 채팅 리스트박스
	g_hListChat = CreateWindowW(L"LISTBOX", NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
		10, 10, 560, 380, hwnd, (HMENU)IDC_LIST_CHAT, NULL, NULL);
	SendMessage(g_hListChat, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 메시지 입력창
	g_hEditMessage = CreateWindowW(L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		10, 300, 300, 10, hwnd, (HMENU)IDC_EDIT_MESSAGE, NULL, NULL);
	SendMessage(g_hEditMessage, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 전송 버튼
	g_hSendButton = CreateWindowW(L"BUTTON", L"전송",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		500, 400, 70, 40, hwnd, (HMENU)IDC_BUTTON_SEND, NULL, NULL);
	SendMessage(g_hSendButton, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 상태바
	g_hStatusBar = CreateWindowW(STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS_BAR, NULL, NULL);

	// 서버 연결 상태 표시
	std::wstring statusText = L"서버: " + StringToWString(serverIP) + L":" + StringToWString(serverPort) + L" (연결 중...)";
	SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());
}
