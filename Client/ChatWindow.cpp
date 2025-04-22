#include "pch.h"
#include "ChatWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "ServerAnalyzer.h"
#include <CommCtrl.h>
#include "WinUtils.h"

// Ŭ���� �̸�
LPCTSTR lpszChatClass = L"ClassChatWindow";

extern ClientServicePtr clientService;
extern string serverIP;
extern string serverPort;

std::map<HWND, ChatWindow*> ChatWindow::s_mapWindow;

LRESULT ChatWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // �ش� �������� ChatWindow ��ü ã��
    ChatWindow* pThis = NULL;
    if (s_mapWindow.find(hwnd) != s_mapWindow.end()) {
        pThis = s_mapWindow[hwnd];
    }

    switch (uMsg)
    {
    case WM_CREATE:
        return 0;

    case WM_COMMAND:
        if (pThis && LOWORD(wParam) == IDC_BUTTON_SEND ||
            (LOWORD(wParam) == IDC_EDIT_MESSAGE && HIWORD(wParam) == VK_RETURN)) {
            pThis->SendChatMessage();
            return 0;
        }
        break;

    case WM_CLIENT_CONNECTED:
        if (pThis) {
            pThis->OnConnect();
            return 0;
        }
        break;

    case WM_CLIENT_DISCONNECT:
        if (pThis) {
            pThis->OnDisconnect();
            return 0;
        }
        break;

    case WM_CLIENT_RECV:
        if (pThis) {
            ChatMessageData* data = (ChatMessageData*)lParam;
            if (data) {
                pThis->OnMessageRecv(data->sender, data->message);
                delete data; // �޸� ����
            }
            return 0;
        }
        break;

    case WM_SIZE:
        if (pThis) {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            // ����Ʈ�ڽ� ũ�� ����
            SetWindowPos(pThis->m_hListChat, NULL,
                10, 10,
                rcClient.right - 20, rcClient.bottom - 90,
                SWP_NOZORDER);

            // �Է�â ũ�� ����
            SetWindowPos(pThis->m_hEditMessage, NULL,
                10, rcClient.bottom - 70,
                rcClient.right - 100, 30,
                SWP_NOZORDER);

            // ���� ��ư ũ�� ����
            SetWindowPos(pThis->m_hSendButton, NULL,
                rcClient.right - 80, rcClient.bottom - 70,
                70, 30,
                SWP_NOZORDER);

            // ���¹� ũ�� ����
            SendMessage(pThis->m_hStatusBar, WM_SIZE, 0, 0);

            return 0;
        }
        break;

    case WM_DESTROY:
        ClientManager::GetInstance().StopClient();
        clientService->CloseService();
        clientService.reset();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ChatWindow::CreateControl()
{
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"���� ���");

    // ä�� ����Ʈ�ڽ�
    m_hListChat = CreateWindowW(L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        10, 10, 560, 380, m_hWnd, (HMENU)IDC_LIST_CHAT, NULL, NULL);
    SendMessage(m_hListChat, WM_SETFONT, (WPARAM)hFont, TRUE);

    // �޽��� �Է�â
    m_hEditMessage = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        10, 400, 560, 30, m_hWnd, (HMENU)IDC_EDIT_MESSAGE, NULL, NULL);
    SendMessage(m_hEditMessage, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ���� ��ư
    m_hSendButton = CreateWindowW(L"BUTTON", L"����",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        580, 400, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_SEND, NULL, NULL);
    SendMessage(m_hSendButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ���¹�
    m_hStatusBar = CreateWindowW(STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, m_hWnd, (HMENU)IDC_STATUS_BAR, NULL, NULL);

    // ���� ���� ���� ǥ��
    std::wstring statusText = L"����: " + WinUtils::StringToWString(serverIP) + L":" + WinUtils::StringToWString(serverPort) + L" (���� ��...)";
    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());
}

ChatWindow::ChatWindow()
	:m_hWnd(NULL), m_hEditMessage(NULL), m_hSendButton(NULL),
	m_hListChat(NULL), m_hStatusBar(NULL), m_isConnect(false)
{
}

ChatWindow::~ChatWindow()
{
	// �� ����
}

bool ChatWindow::Init(HINSTANCE hInstance)
{
    // ��Ʈ�� �ʱ�ȭ
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // ������ Ŭ���� ���
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ChatWindow::WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = lpszChatClass;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        MessageBoxW(NULL, L"ä�� ������ Ŭ���� ��� ����", L"����", MB_ICONERROR);
        return false;
    }

    // ������ ����
    m_hWnd = CreateWindowW(lpszChatClass, L"ä��", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"ä�� ������ ���� ����", L"����", MB_ICONERROR);
        return false;
    }

    ClientSession::SetChatWin(m_hWnd);
    // ��ü ����
    s_mapWindow[m_hWnd] = this;

    // ���ǿ� ������ �ڵ� ����
    //ClientSession::SetMainWin(m_hWnd);

    // ��Ʈ�� ����
    CreateControl();

    return true;
}

void ChatWindow::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
}

void ChatWindow::Hide()
{
    ShowWindow(m_hWnd, SW_HIDE);
}

bool ChatWindow::IsVisible() const
{
    return IsWindowVisible(m_hWnd);
}

void ChatWindow::OnConnect()
{
    UpdateStatus(true);
    AddChatMessage(L"������ ����Ǿ����ϴ�.");
}

void ChatWindow::OnDisconnect()
{
    UpdateStatus(false);
    AddChatMessage(L"������ ������ ���������ϴ�.");
}

void ChatWindow::OnMessageRecv(const std::string& sender, const std::string& message)
{
    std::wstring displayMessage = WinUtils::StringToWString(sender + ": " + message);
    AddChatMessage(displayMessage);
}

void ChatWindow::SendChatMessage()
{
    if (!m_isConnect)
    {
        MessageBoxW(m_hWnd, L"������ ����Ǿ����� �ʽ��ϴ�.", L"����", MB_ICONERROR);
        return;
    }

    // ������Ʈ�ѿ��� �ؽ�Ʈ ��������
    int length = GetWindowTextLengthW(m_hEditMessage);
    if (length == 0)
        return;

    std::vector<wchar_t> buffer(length + 1);
    GetWindowTextW(m_hEditMessage, buffer.data(), length + 1);
    std::wstring wMessage(buffer.data());
    std::string message = WinUtils::WStringToString(wMessage);

    // �޽��� ����
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            //static_cast<ClientSession*>(session.get())->Send(message, PacketType::ChatReq);

            // ���� ���� �޽��� ǥ��
            std::wstring myMessage = L"��: " + wMessage;
            AddChatMessage(myMessage);

            // �Է�â �ʱ�ȭ
            SetWindowTextW(m_hEditMessage, L"");
        }
    }
}

void ChatWindow::AddChatMessage(const std::wstring& message)
{
    // ä�� �޽��� ����
    {
        std::lock_guard<std::mutex> lock(m_chatMutex);
        m_chatMessage.push_back(message);

        // �ִ� �޽��� �� ����
        while (m_chatMessage.size() > 100)
        {
            m_chatMessage.erase(m_chatMessage.begin());
        }
    }

    // ���� �ð� ��������
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
    localtime_s(&local_time, &now_time_t);

    // �ð��� �޽��� ����
    std::wstringstream wss;
    wss << L"[" << local_time.tm_year + 1900 << L"-"
        << std::setfill(L'0') << std::setw(2) << local_time.tm_mon + 1 << L"-"
        << std::setfill(L'0') << std::setw(2) << local_time.tm_mday << L" "
        << std::setfill(L'0') << std::setw(2) << local_time.tm_hour << L":"
        << std::setfill(L'0') << std::setw(2) << local_time.tm_min << L":"
        << std::setfill(L'0') << std::setw(2) << local_time.tm_sec << L"] "
        << message;

    std::wstring chatMessage = wss.str();

    // ����Ʈ �ڽ��� �߰�
    SendMessage(m_hListChat, LB_ADDSTRING, 0, (LPARAM)chatMessage.c_str());

    // �ڵ� ��ũ��
    int count = SendMessage(m_hListChat, LB_GETCOUNT, 0, 0);
    SendMessage(m_hListChat, LB_SETTOPINDEX, count - 1, 0);
}

void ChatWindow::UpdateStatus(bool isConnect)
{
    m_isConnect = isConnect;

    std::wstring statusText;
    if (isConnect) {
        statusText = L"����: " + WinUtils::StringToWString(serverIP) + L":" + WinUtils::StringToWString(serverPort) + L" (�����)";
    }
    else {
        statusText = L"����: " + WinUtils::StringToWString(serverIP) + L":" + WinUtils::StringToWString(serverPort) + L" (���� ����)";
    }

    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());
    EnableWindow(m_hSendButton, isConnect);
}

void ChatWindow::SetTitle(const std::wstring& title)
{
    SetWindowTextW(m_hWnd, title.c_str());
}
