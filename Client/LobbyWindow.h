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

    // 초기화 및 창 표시
    bool Init(HINSTANCE hInstance);
    void Show(const std::string& userId);
    void Hide();
    bool IsVisible() const;
    void SetMainWnd(HWND hwnd) { m_hParentHandle = hwnd; }

    // 이벤트 처리
    void RefreshRoomList();
    void EnterChatRoom(int roomId);
    void CreateNewChatRoom(const std::string& roomName);

    // 채팅방 목록 갱신
    void UpdateLobby(const std::vector<ChatRoomInfo>& roomList);

    // 창 핸들 얻기
    HWND GetHandle() const { return m_hWnd; }

private:
    // 윈도우 핸들 및 컨트롤
    HWND m_hWnd;                   // 메인 윈도우 핸들
    HWND m_hRoomList;              // 채팅방 목록 리스트박스
    HWND m_hEnterButton;           // 입장 버튼
    HWND m_hCreateButton;          // 생성 버튼
    HWND m_hRoomNameEdit;          // 채팅방 이름 입력 필드
    HWND m_hWelcomeLabel;          // 환영 메시지 라벨

    HWND m_hParentHandle;

    std::string m_currentUserId;   // 현재 로그인한 사용자 ID
    std::vector<ChatRoomInfo> m_chatRooms;  // 채팅방 목록 데이터

    // 윈도우 프로시저 (static으로 선언해야 Win32 API에서 사용 가능)
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // 윈도우 핸들과 객체 연결을 위한 맵
    static std::map<HWND, LobbyWindow*> s_mapWindow;

    // 컨트롤 생성
    void CreateControl();
};

