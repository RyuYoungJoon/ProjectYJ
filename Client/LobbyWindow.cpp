#include "pch.h"
#include "LobbyWindow.h"
#include "ClientManager.h"
#include "ClientSession.h"
#include "WinUtils.h"

LPCTSTR lpszLobbyClass = L"ClassLobbyWindow";

std::map<HWND, LobbyWindow*> LobbyWindow::s_mapWindow;

LobbyWindow::LobbyWindow()
	: m_hWnd(NULL), m_hRoomList(NULL), m_hEnterButton(NULL),
	m_hCreateButton(NULL), m_hRoomNameEdit(NULL), m_hWelcomeLabel(NULL)
{
}

LobbyWindow::~LobbyWindow()
{
	/*if (m_hWnd)
	{
        s_mapWindow.clear();
	}*/
}

bool LobbyWindow::Init(HINSTANCE hInstance)
{
    // ������ Ŭ���� ���
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = LobbyWindow::WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = lpszLobbyClass;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        MessageBoxW(NULL, L"ä�ù� ��� ������ Ŭ���� ��� ����", L"����", MB_ICONERROR);
        return false;
    }

    // ������ ����
    m_hWnd = CreateWindowW(lpszLobbyClass, L"ä�ù� ���", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500, m_hParentHandle, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"ä�ù� ��� ������ ���� ����", L"����", MB_ICONERROR);
        return false;
    }

    ClientSession::SetLobbyWin(m_hWnd);
    // ��ü ����
    s_mapWindow[m_hWnd] = this;

    // ��Ʈ�� ����
    CreateControl();

    return true;
}

void LobbyWindow::Show(const std::string& userId)
{
    m_currentUserId = userId;

    // ȯ�� �޽��� ������Ʈ
    std::wstring welcomeMsg = L"�ȳ��ϼ���, " + WinUtils::StringToWString(userId) + L"��! ä�ù��� �����ϰų� ���� ��������.";
    SetWindowTextW(m_hWelcomeLabel, welcomeMsg.c_str());

    // ä�ù� ��� ������ ��û (�׽�Ʈ �����ͷ� ��ü)
    // �����δ� ������ ��û�� ������ ������ �޾ƾ� ��
    /*std::vector<ChatRoomInfo> testRooms = {
        {1, "�Ϲ� ��ȭ��", 5, 20},
        {2, "���� ��й�", 10, 30},
        {3, "���� �����", 3, 10}
    };
    UpdateLobby(testRooms);*/

    // ä�ù� ��� ��û ��Ŷ ����
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            //PacketRoomListReq packet;
            //static_cast<ClientSession*>(session.get())->Send(packet);
        }
    }

    // â ǥ��
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
}

void LobbyWindow::Hide()
{
    ShowWindow(m_hWnd, SW_HIDE);
}

bool LobbyWindow::IsVisible() const
{
    return IsWindowVisible(m_hWnd);
}

void LobbyWindow::RefreshRoomList()
{
    // ������ ä�ù� ��� ��û ��Ŷ ����
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            //PacketRoomListReq packet;
           // static_cast<ClientSession*>(session.get())->Send(packet);
        }
    }
}

void LobbyWindow::EnterChatRoom(int roomId)
{
    // ������ ä�ù濡 �ش��ϴ� ���� ã��
    ChatRoomInfo selectedRoom;
    bool found = false;

    for (const auto& room : m_chatRooms)
    {
        if (room.roomID == roomId)
        {
            selectedRoom = room;
            found = true;
            break;
        }
    }

    if (!found)
    {
        MessageBoxW(m_hWnd, L"������ ä�ù��� ã�� �� �����ϴ�.", L"����", MB_ICONERROR);
        return;
    }

    // ä�ù� ���� Ȯ�� �޽���
    //std::wstring message = L"'" + WinUtils::StringToWString(selectedRoom.roomName) +
    //    L"' ä�ù濡 �����Ͻðڽ��ϱ�?";

    if (MessageBoxW(m_hWnd, L"", L"ä�ù� ����", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        // ������ ä�ù� ���� ��û ��Ŷ ����
        auto& ClientManager = ClientManager::GetInstance();
        auto sessions = ClientManager.GetSessions();

        if (!sessions.empty())
        {
            auto session = sessions.begin()->second;
            if (session)
            {
                // ä�ù� ���� ��û ��Ŷ ����
                //static_cast<ClientSession*>(session.get())->Send(std::to_string(roomId), PacketType::RoomEnterReq);

                // ä�ù� ���� ó���� WM_CLIENT_CHATROOM_ENTER �޽��� �ڵ鷯���� ó��
                //PacketRoomEnterReq packet;
                //packet.payload.roomID = selectedRoom.roomID;

                //static_cast<ClientSession*>(session.get())->Send(packet);

                // �׽�Ʈ�� ���� �ӽ� �ڵ� - �����δ� ���� ������ ��ٸ�
                Hide();
            }
            else
            {
                MessageBoxW(m_hWnd, L"������ ��ȿ���� �ʽ��ϴ�.", L"����", MB_ICONERROR);
            }
        }
        else
        {
            MessageBoxW(m_hWnd, L"������ ����Ǿ� ���� �ʽ��ϴ�.", L"����", MB_ICONERROR);
        }
    }
}

void LobbyWindow::CreateNewChatRoom(const std::string& roomName)
{
    // ������ ä�ù� ���� ��û ��Ŷ ����
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            // ä�ù� ���� ��û ��Ŷ ���� (format: "roomName|maxUsers")
            //PacketRoomCreateReq packet;
            //packet.payload.roomName = roomName;
            //static_cast<ClientSession*>(session.get())->Send(packet);

            // ä�ù� ���� ó���� WM_CLIENT_CHATROOM_CREATE �޽��� �ڵ鷯���� ó��
        }
    }

    //// �׽�Ʈ�� ���� �ӽ� ó��
    //MessageBoxA(m_hWnd, (roomName + " ä�ù��� �����Ǿ����ϴ�.").c_str(), "ä�ù� ����", MB_ICONINFORMATION);

    //// ���� ������ ä�ù��� ��Ͽ� �߰� (�����δ� �������� ����� �ٽ� �޾ƾ� ��)
    //ChatRoomInfo newRoom = {
    //    (int)m_chatRooms.size() + 1,  // �ӽ� ID
    //    roomName,
    //    1,  // �ڱ� �ڽ�
    //    20  // �⺻ �ִ� �ο�
    //};

    //m_chatRooms.push_back(newRoom);
    //UpdateLobby(m_chatRooms);
}

void LobbyWindow::UpdateLobby(const std::vector<ChatRoomInfo>& roomList)
{
    m_chatRooms = roomList;

    // ����Ʈ�ڽ� ���� �ʱ�ȭ
    SendMessage(m_hRoomList, LB_RESETCONTENT, 0, 0);

    // �� ä�ù� ������ ����Ʈ�ڽ��� �߰�
    for (const auto& room : m_chatRooms)
    {
        std::wstring itemText = /*WinUtils::StringToWString(room.roomID) +*/ L" (" + std::to_wstring(room.currentUser) +
            L"/" + std::to_wstring(room.maxUser) + L")";
        SendMessage(m_hRoomList, LB_ADDSTRING, 0, (LPARAM)itemText.c_str());
    }
}

LRESULT LobbyWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // �ش� �������� ChatRoomList ��ü ã��
    LobbyWindow* pThis = NULL;
    if (s_mapWindow.find(hwnd) != s_mapWindow.end()) {
        pThis = s_mapWindow[hwnd];
    }

    switch (uMsg) {
    case WM_CREATE:
        return 0;

    case WM_COMMAND:
        if (pThis) {
            if (LOWORD(wParam) == IDC_BUTTON_ENTER) {
                // ���õ� ä�ù� �ε��� ��������
                int selectedIndex = SendMessage(pThis->m_hRoomList, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR && selectedIndex < pThis->m_chatRooms.size()) {
                    pThis->EnterChatRoom(pThis->m_chatRooms[selectedIndex].roomID);
                }
                else {
                    MessageBoxW(hwnd, L"ä�ù��� �������ּ���.", L"�˸�", MB_ICONINFORMATION);
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_BUTTON_CREATE) {
                // �� ä�ù� �̸� ��������
                int nameLength = GetWindowTextLengthW(pThis->m_hRoomNameEdit);
                if (nameLength > 0) {
                    std::vector<wchar_t> buffer(nameLength + 1);
                    GetWindowTextW(pThis->m_hRoomNameEdit, buffer.data(), nameLength + 1);
                    std::wstring wRoomName(buffer.data());

                    // ä�ù� �̸��� UTF-8�� ��ȯ
                    std::string roomName = WinUtils::WStringToString(wRoomName);

                    // �� ä�ù� ����
                    pThis->CreateNewChatRoom(roomName);

                    // �Է� �ʵ� �ʱ�ȭ
                    SetWindowTextW(pThis->m_hRoomNameEdit, L"");
                }
                else {
                    MessageBoxW(hwnd, L"ä�ù� �̸��� �Է����ּ���.", L"�˸�", MB_ICONINFORMATION);
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_BUTTON_REFRESH) {
                pThis->RefreshRoomList();
                return 0;
            }
        }
        break;

    case WM_CLIENT_CHATROOM_LIST:
        if (pThis) {
            // ä�ù� ��� ���� ó��
            ChatRoomListResponseData* data = (ChatRoomListResponseData*)lParam;
            if (data) 
            {
                // ä�ù� ��� ������Ʈ
                std::vector<ChatRoomInfo> roomList;
                for (const auto& roomData : data->rooms) {
                    ChatRoomInfo roomInfo;
                    roomInfo.roomID = roomData.roomID;
                    //roomInfo.roomName = roomData.roomName;
                    roomInfo.currentUser = roomData.currentUser;
                    roomInfo.maxUser = roomData.maxUser;
                    roomList.push_back(roomInfo);
                }

                pThis->UpdateLobby(roomList);

                // �޸� ����
                delete data;
            }
        }
        break;

    case WM_CLIENT_CHATROOM_ENTER:
        if (pThis) {
            // ä�ù� ���� ���� ó��
            ChatRoomResponseData* data = (ChatRoomResponseData*)lParam;
            if (data)
            {
                // ä�ù� ��� â �����
                pThis->Hide();

                // ä��â���� ��ȯ �޽��� ����
                ::PostMessage(GetParent(hwnd), WM_ENTER_CHATROOM, 0, lParam);
                // lParam�� �״�� �����ϹǷ� ���⼭�� �޸� �������� ����
            }
            else
            {
                //MessageBoxW(hwnd, WinUtils::StringToWString(data->message).c_str(), L"ä�ù� ���� ����", MB_ICONERROR);
                // ������ ��쿡�� �޸� ����
                delete data;

            }
        }
        break;
    case WM_CLIENT_CHATROOM_CREATE:
        if (pThis)
        {
            // ä�ù� ���� ���� ó��
            ChatRoomResponseData* data = (ChatRoomResponseData*)lParam;
            if (data)
            {
                MessageBoxW(hwnd, WinUtils::StringToWString("ä�ù� '" + data->roomName + "'��(��) �����Ǿ����ϴ�.").c_str(), L"ä�ù� ���� ����", MB_ICONINFORMATION);

                ChatRoomInfo newRoom = { data->roomId, 1, 10 };

                pThis->m_chatRooms.push_back(newRoom);
                pThis->UpdateLobby(pThis->m_chatRooms);
                // ��� ���ΰ�ħ ��û
                pThis->RefreshRoomList();
            }
            else {
                //MessageBoxW(hwnd, WinUtils::StringToWString(data->message).c_str(), L"ä�ù� ���� ����", MB_ICONERROR);
            }

            // �޸� ����
            delete data;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void LobbyWindow::CreateControl()
{
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"���� ���");

    // ȯ�� �޽��� ��
    m_hWelcomeLabel = CreateWindowW(L"STATIC", L"ä�ù� ���",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        20, 20, 560, 25, m_hWnd, (HMENU)IDC_STATIC_WELCOME, NULL, NULL);
    SendMessage(m_hWelcomeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ä�ù� ��� ����Ʈ�ڽ�
    m_hRoomList = CreateWindowW(L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        20, 60, 560, 300, m_hWnd, (HMENU)IDC_LIST_ROOMS, NULL, NULL);
    SendMessage(m_hRoomList, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ä�ù� ���� ��ư
    m_hEnterButton = CreateWindowW(L"BUTTON", L"����",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        450, 370, 130, 30, m_hWnd, (HMENU)IDC_BUTTON_ENTER, NULL, NULL);
    SendMessage(m_hEnterButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // �� ä�ù� �̸� ��
    HWND hStaticRoomName = CreateWindowW(L"STATIC", L"�� ä�ù� �̸�:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 380, 130, 20, m_hWnd, (HMENU)IDC_STATIC_ROOM_NAME, NULL, NULL);
    SendMessage(hStaticRoomName, WM_SETFONT, (WPARAM)hFont, TRUE);

    // �� ä�ù� �̸� �Է� �ʵ�
    m_hRoomNameEdit = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        150, 380, 200, 25, m_hWnd, (HMENU)IDC_EDIT_ROOM_NAME, NULL, NULL);
    SendMessage(m_hRoomNameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // �� ä�ù� ���� ��ư
    m_hCreateButton = CreateWindowW(L"BUTTON", L"����",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 420, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_CREATE, NULL, NULL);
    SendMessage(m_hCreateButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // ���ΰ�ħ ��ư
    HWND hRefreshButton = CreateWindowW(L"BUTTON", L"���ΰ�ħ",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 420, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_REFRESH, NULL, NULL);
    SendMessage(hRefreshButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}
