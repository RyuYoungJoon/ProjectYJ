#pragma once

#define IDC_LIST_ROOMS 3001
#define IDC_BUTTON_ENTER 3002
#define IDC_BUTTON_CREATE 3003
#define IDC_EDIT_ROOM_NAME 3004
#define IDC_STATIC_WELCOME 3005
#define IDC_STATIC_ROOM_NAME 3006
#define IDC_BUTTON_REFRESH 3007


class LobbyWindow
{
public:
    LobbyWindow();
    ~LobbyWindow();

    // �ʱ�ȭ �� â ǥ��
    bool Init(HINSTANCE hInstance);
    void Show(const std::string& userId);
    void Hide();
    bool IsVisible() const;
    void SetMainWnd(HWND hwnd) { m_hParentHandle = hwnd; }

    // �̺�Ʈ ó��
    void RefreshRoomList();
    void EnterChatRoom(int roomId);
    void CreateNewChatRoom(const std::string& roomName);

    // ä�ù� ��� ����
    void UpdateLobby(const std::vector<ChatRoomInfo>& roomList);

    // â �ڵ� ���
    HWND GetHandle() const { return m_hWnd; }
    std::vector<ChatRoomInfo> m_chatRooms;  // ä�ù� ��� ������

private:
    // ������ �ڵ� �� ��Ʈ��
    HWND m_hWnd;                   // ���� ������ �ڵ�
    HWND m_hRoomList;              // ä�ù� ��� ����Ʈ�ڽ�
    HWND m_hEnterButton;           // ���� ��ư
    HWND m_hCreateButton;          // ���� ��ư
    HWND m_hRoomNameEdit;          // ä�ù� �̸� �Է� �ʵ�
    HWND m_hWelcomeLabel;          // ȯ�� �޽��� ��

    HWND m_hParentHandle;

    std::string m_currentUserId;   // ���� �α����� ����� ID

    // ������ ���ν��� (static���� �����ؾ� Win32 API���� ��� ����)
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // ������ �ڵ�� ��ü ������ ���� ��
    static std::map<HWND, LobbyWindow*> s_mapWindow;

    // ��Ʈ�� ����
    void CreateControl();
};

