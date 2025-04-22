#include "pch.h"
#include "LoginWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "WinUtils.h"

LPCTSTR lpszLoginClass = L"ClassLoginWindow";
std::map<HWND, LoginWindow*> LoginWindow::s_mapWindow;

LoginWindow::LoginWindow()
	: m_hWnd(NULL), m_hEditId(NULL), m_hEditPassword(NULL), m_hLoginButton(NULL)
{
}

LoginWindow::~LoginWindow()
{
	if (m_hWnd)
	{
		s_mapWindow.erase(m_hWnd);
	}
}

bool LoginWindow::Init(HINSTANCE hInstance)
{
    // 윈도우 클래스 등록
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
        MessageBoxW(NULL, L"로그인 윈도우 클래스 등록 실패", L"오류", MB_ICONERROR);
        return false;
    }

    // 윈도우 생성
    m_hWnd = CreateWindowW(lpszLoginClass, L"로그인", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, m_hParentHandle, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"로그인 윈도우 생성 실패", L"오류", MB_ICONERROR);
        return false;
    }

    // 객체 연결
    s_mapWindow[m_hWnd] = this;

    // 컨트롤 생성
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
    // ID와 비밀번호 가져오기
    int idLength = GetWindowTextLengthW(m_hEditId);
    int pwLength = GetWindowTextLengthW(m_hEditPassword);

    if (idLength == 0 || pwLength == 0)
    {
        MessageBoxW(m_hWnd, L"아이디와 비밀번호를 모두 입력해주세요.", L"로그인 실패", MB_ICONERROR);
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

    // 로그인 요청 로직
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            // 로그인 요청 패킷 전송
            PacketLoginReq packet;
            packet.header.type = PacketType::LoginReq;
            packet.payload.id = id; 
            packet.payload.password = password;
            static_cast<ClientSession*>(session.get())->Send(packet);

            // 서버 응답을 기다림 (WM_CLIENT_LOGIN 메시지로 처리)
            // 테스트를 위해 임시로 로그인 성공으로 처리
            LoginResponseData* testData = new LoginResponseData();
            testData->result = LOGIN_SUCCESS;
            testData->userId = id;
            testData->message = "로그인 성공";

            ::PostMessage(m_hWnd, WM_CLIENT_LOGIN, 0, (LPARAM)testData);
        }
        else
        {
            OnLoginFail("세션이 유효하지 않습니다.");
        }
    }
    else
    {
        OnLoginFail("서버에 연결되어 있지 않습니다. 다시 시도해주세요.");
    }
}

void LoginWindow::OnLoginSuccess(const std::string& userID)
{
    m_UserID = userID;
    Hide();

    extern HWND g_hMainWnd;

    // 여기서 채팅방 목록 창을 띄우는 로직은 Client.cpp에서 처리할 예정
    // LoginWindow는 단순히 로그인 성공 이벤트만 발생시킴
    // WM_LOGIN_SUCCESS 메시지를 메인 윈도우에게 보내 처리하도록 함
    ::PostMessage(m_hParentHandle, WM_LOGIN_SUCCESS, 0, 0);
}

void LoginWindow::OnLoginFail(const std::string& error)
{
    MessageBoxA(m_hWnd, error.c_str(), "ERROR", MB_ICONERROR);
}

LRESULT LoginWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 해당 윈도우의 ChatWindow 객체 찾기
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

    case WM_CLIENT_LOGIN:
    {
        // 로그인 응답 처리
        LoginResponseData* data = (LoginResponseData*)lParam;
        if (data) {
            if (data->result == LOGIN_SUCCESS) {
                pThis->OnLoginSuccess(data->userId);
            }
            else {
                pThis->OnLoginFail(data->message);
            }

            // 메모리 해제
            delete data;
        }
    }
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
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");

    // ID 라벨
    HWND hStaticID = CreateWindowW(L"STATIC", L"아이디:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        50, 50, 80, 20, m_hWnd, (HMENU)IDC_STATIC_ID, NULL, NULL);
    SendMessage(hStaticID, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ID 입력 필드
    m_hEditId = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        150, 50, 200, 25, m_hWnd, (HMENU)IDC_EDIT_ID, NULL, NULL);
    SendMessage(m_hEditId, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 비밀번호 라벨
    HWND hStaticPassword = CreateWindowW(L"STATIC", L"비밀번호:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        50, 100, 80, 20, m_hWnd, (HMENU)IDC_STATIC_PASSWORD, NULL, NULL);
    SendMessage(hStaticPassword, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 비밀번호 입력 필드
    m_hEditPassword = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
        150, 100, 200, 25, m_hWnd, (HMENU)IDC_EDIT_PASSWORD, NULL, NULL);
    SendMessage(m_hEditPassword, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 로그인 버튼
    m_hLoginButton = CreateWindowW(L"BUTTON", L"로그인",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        150, 150, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_LOGIN, NULL, NULL);
    SendMessage(m_hLoginButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}
