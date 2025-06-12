#pragma once
#include "ServerPacketHandler.h"

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

    bool Init(HINSTANCE hInstance);
    void Show(const std::string& userId);
    void Hide();
    bool IsVisible() const;
    void SetMainWnd(HWND hwnd) { m_hParentHandle = hwnd; }

    void RefreshRoomList();
    void EnterChatRoom(int roomId);
    void CreateNewChatRoom(const std::string& roomName);

    void UpdateLobby(const std::vector<Protocol::ChatRoomInfo>& roomList);

    HWND GetHandle() const { return m_hWnd; }
    std::vector<Protocol::ChatRoomInfo> m_chatRooms;

private:
    HWND m_hWnd;                   
    HWND m_hRoomList;              
    HWND m_hEnterButton;           
    HWND m_hCreateButton;          
    HWND m_hRoomNameEdit;          
    HWND m_hWelcomeLabel;          

    HWND m_hParentHandle;

    std::string m_currentUserId;   

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static std::map<HWND, LobbyWindow*> s_mapWindow;

    void CreateControl();
};

