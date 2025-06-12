#include "pch.h"
#include "LoginWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "WinUtils.h"
#include "ServerPacketHandler.h"
#include "Protocol.pb.h"

LPCTSTR lpszLoginClass = L"ClassLoginWindow";
std::map<HWND, LoginWindow*> LoginWindow::s_mapWindow;

LoginWindow::LoginWindow()
	: m_hWnd(NULL), m_hEditId(NULL), m_hEditPassword(NULL), m_hLoginButton(NULL)
{
}

LoginWindow::~LoginWindow()
{
	
}

bool LoginWindow::Init(HINSTANCE hInstance)
{
    // ������ Ŭ���� ���
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = LoginWindow::WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = lpszLoginClass;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        MessageBoxW(NULL, L"�α��� ������ Ŭ���� ��� ����", L"����", MB_ICONERROR);
        return false;
    }

    // ������ ����
    m_hWnd = CreateWindowW(lpszLoginClass, L"�α���", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, m_hParentHandle, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"�α��� ������ ���� ����", L"����", MB_ICONERROR);
        return false;
    }

    // ��ü ����
    s_mapWindow[m_hWnd] = this;

    // ��Ʈ�� ����
    CreateControl();

    return true;
}

void LoginWindow::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
}

void LoginWindow::Hide()
{
    ShowWindow(m_hWnd, SW_HIDE);
}

bool LoginWindow::IsVisible() const
{
    return IsWindowVisible(m_hWnd);
}

void LoginWindow::TryLogin()
{
    // ID�� ��й�ȣ ��������
    int idLength = GetWindowTextLengthW(m_hEditId);
    int pwLength = GetWindowTextLengthW(m_hEditPassword);

    if (idLength == 0 || pwLength == 0)
    {
        MessageBoxW(m_hWnd, L"���̵�� ��й�ȣ�� ��� �Է����ּ���.", L"�α��� ����", MB_ICONERROR);
        return;
    }

    std::vector<wchar_t> idBuffer(idLength + 1);
    std::vector<wchar_t> pwBuffer(pwLength + 1);

    GetWindowTextW(m_hEditId, idBuffer.data(), idLength + 1);
    GetWindowTextW(m_hEditPassword, pwBuffer.data(), pwLength + 1);

    std::wstring wId(idBuffer.data());
    std::wstring wPassword(pwBuffer.data());

    std::string id = WinUtils::WStringToString(wId);
    std::string password = WinUtils::WStringToString(wPassword);

    // �α��� ��û ����
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            Protocol::LoginReq loginPacket;
            loginPacket.set_id(std::stoi(id));
            loginPacket.set_password(std::stoi(password));
            
            Packet sendPacket = PacketHandler::GetInstance().MakePacket(loginPacket);

            static_cast<ClientSession*>(session.get())->Send(std::move(sendPacket));
        }
        else
        {
            OnLoginFail("������ ��ȿ���� �ʽ��ϴ�.");
        }
    }
    else
    {
        OnLoginFail("������ ����Ǿ� ���� �ʽ��ϴ�. �ٽ� �õ����ּ���.");
    }
}

void LoginWindow::OnLoginSuccess(const std::string& userID)
{
    m_UserID = userID;
    Hide();
}

void LoginWindow::OnLoginFail(const std::string& error)
{
    MessageBoxA(m_hWnd, error.c_str(), "ERROR", MB_ICONERROR);
}

LRESULT LoginWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // �ش� �������� ChatWindow ��ü ã��
    LoginWindow* pThis = NULL;
    if (s_mapWindow.find(hwnd) != s_mapWindow.end()) {
        pThis = s_mapWindow[hwnd];
    }
    switch (uMsg) {
    case WM_CREATE:
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_LOGIN) {
            pThis->TryLogin();
        }
        break;
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void LoginWindow::CreateControl()
{
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"���� ���");

    // ID ��
    HWND hStaticID = CreateWindowW(L"STATIC", L"���̵�:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        50, 50, 80, 20, m_hWnd, (HMENU)IDC_STATIC_ID, NULL, NULL);
    SendMessage(hStaticID, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ID �Է� �ʵ�
    m_hEditId = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        150, 50, 200, 25, m_hWnd, (HMENU)IDC_EDIT_ID, NULL, NULL);
    SendMessage(m_hEditId, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ��й�ȣ ��
    HWND hStaticPassword = CreateWindowW(L"STATIC", L"��й�ȣ:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        50, 100, 80, 20, m_hWnd, (HMENU)IDC_STATIC_PASSWORD, NULL, NULL);
    SendMessage(hStaticPassword, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ��й�ȣ �Է� �ʵ�
    m_hEditPassword = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
        150, 100, 200, 25, m_hWnd, (HMENU)IDC_EDIT_PASSWORD, NULL, NULL);
    SendMessage(m_hEditPassword, WM_SETFONT, (WPARAM)hFont, TRUE);

    // �α��� ��ư
    m_hLoginButton = CreateWindowW(L"BUTTON", L"�α���",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        150, 150, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_LOGIN, NULL, NULL);
    SendMessage(m_hLoginButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}
