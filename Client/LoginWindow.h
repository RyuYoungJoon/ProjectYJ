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

    // �̺�Ʈ ó��
    void TryLogin();
    void OnLoginSuccess(const std::string& userID);
    void OnLoginFail(const std::string& error);

    void SetMainWnd(HWND hwnd) { m_hParentHandle = hwnd; }
    
    // �ڵ� ���
    HWND GetHandle() const { return m_hWnd; }

    const std::string& GetUserID() const { return m_UserID; }

private:
    // ������ �ڵ� �� ��Ʈ��
    HWND m_hWnd;               // ���� ������ �ڵ�
    HWND m_hEditId;            // ID �Է� �ʵ�
    HWND m_hEditPassword;      // ��й�ȣ �Է� �ʵ�
    HWND m_hLoginButton;       // �α��� ��ư

    HWND m_hParentHandle;

    std::string m_UserID;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static std::map<HWND, LoginWindow*> s_mapWindow;

    void CreateControl();
};

