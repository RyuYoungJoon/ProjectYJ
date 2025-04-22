#pragma once

#define IDC_EDIT_ID 2001
#define IDC_EDIT_PASSWORD 2002
#define IDC_BUTTON_LOGIN 2003
#define IDC_STATIC_ID 2004
#define IDC_STATIC_PASSWORD 2005

class LoginWindow
{
public:
    LoginWindow();
    ~LoginWindow();

    bool Init(HINSTANCE hInstance);
    void Show();
    void Hide();
    bool IsVisible() const;

    // 이벤트 처리
    void TryLogin();
    void OnLoginSuccess(const std::string& userID);
    void OnLoginFail(const std::string& error);

    void SetMainWnd(HWND hwnd) { m_hParentHandle = hwnd; }
    
    // 핸들 얻기
    HWND GetHandle() const { return m_hWnd; }

    const std::string& GetUserID() const { return m_UserID; }

private:
    // 윈도우 핸들 및 컨트롤
    HWND m_hWnd;               // 메인 윈도우 핸들
    HWND m_hEditId;            // ID 입력 필드
    HWND m_hEditPassword;      // 비밀번호 입력 필드
    HWND m_hLoginButton;       // 로그인 버튼

    HWND m_hParentHandle;

    std::string m_UserID;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static std::map<HWND, LoginWindow*> s_mapWindow;

    void CreateControl();
};

