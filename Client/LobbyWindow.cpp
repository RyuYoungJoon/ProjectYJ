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
    // 윈도우 클래스 등록
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
        MessageBoxW(NULL, L"채팅방 목록 윈도우 클래스 등록 실패", L"오류", MB_ICONERROR);
        return false;
    }

    m_hWnd = CreateWindowW(lpszLobbyClass, L"채팅방 목록", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500, m_hParentHandle, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"채팅방 목록 윈도우 생성 실패", L"오류", MB_ICONERROR);
        return false;
    }

    ClientSession::SetLobbyWin(m_hWnd);
    s_mapWindow[m_hWnd] = this;

    CreateControl();

    return true;
}

void LobbyWindow::Show(const std::string& userId)
{
    m_currentUserId = userId;

    // 환영 메시지 업데이트
    std::wstring welcomeMsg = L"안녕하세요, " + WinUtils::StringToWString(userId) + L"님! 채팅방을 선택하거나 새로 만들어보세요.";
    SetWindowTextW(m_hWelcomeLabel, welcomeMsg.c_str());

    // 채팅방 목록 데이터 요청 (테스트 데이터로 대체)
    // 실제로는 서버에 요청을 보내고 응답을 받아야 함
    /*std::vector<ChatRoomInfo> testRooms = {
        {1, "일반 대화방", 5, 20},
        {2, "게임 토론방", 10, 30},
        {3, "음악 감상방", 3, 10}
    };
    UpdateLobby(testRooms);*/

    // 채팅방 목록 요청 패킷 전송
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

    // 창 표시
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
    // 서버에 채팅방 목록 요청 패킷 전송
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
}

void LobbyWindow::EnterChatRoom(int roomId)
{
    // 선택한 채팅방에 해당하는 정보 찾기
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
        MessageBoxW(m_hWnd, L"선택한 채팅방을 찾을 수 없습니다.", L"오류", MB_ICONERROR);
        return;
    }

    // 채팅방 입장 확인 메시지
    //std::wstring message = L"'" + WinUtils::StringToWString(selectedRoom.roomName) +
    //    L"' 채팅방에 입장하시겠습니까?";

    if (MessageBoxW(m_hWnd, L"", L"채팅방 입장", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        // 서버에 채팅방 입장 요청 패킷 전송
        auto& ClientManager = ClientManager::GetInstance();
        auto sessions = ClientManager.GetSessions();

        if (!sessions.empty())
        {
            auto session = sessions.begin()->second;
            if (session)
            {
                // 채팅방 입장 요청 패킷 전송
                //static_cast<ClientSession*>(session.get())->Send(std::to_string(roomId), PacketType::RoomEnterReq);

                // 채팅방 입장 처리는 WM_CLIENT_CHATROOM_ENTER 메시지 핸들러에서 처리
                //PacketRoomEnterReq packet;
                //packet.payload.roomID = selectedRoom.roomID;

                //static_cast<ClientSession*>(session.get())->Send(packet);

                // 테스트를 위한 임시 코드 - 실제로는 서버 응답을 기다림
                Hide();
            }
            else
            {
                MessageBoxW(m_hWnd, L"세션이 유효하지 않습니다.", L"오류", MB_ICONERROR);
            }
        }
        else
        {
            MessageBoxW(m_hWnd, L"서버에 연결되어 있지 않습니다.", L"오류", MB_ICONERROR);
        }
    }
}

void LobbyWindow::CreateNewChatRoom(const std::string& roomName)
{
    // 서버에 채팅방 생성 요청 패킷 전송
    auto& ClientManager = ClientManager::GetInstance();
    auto sessions = ClientManager.GetSessions();

    if (!sessions.empty())
    {
        auto session = sessions.begin()->second;
        if (session)
        {
            // 채팅방 생성 요청 패킷 전송 (format: "roomName|maxUsers")
            //PacketRoomCreateReq packet;
            //packet.payload.roomName = roomName;
            //static_cast<ClientSession*>(session.get())->Send(packet);

            // 채팅방 생성 처리는 WM_CLIENT_CHATROOM_CREATE 메시지 핸들러에서 처리
        }
    }

    //// 테스트를 위한 임시 처리
    //MessageBoxA(m_hWnd, (roomName + " 채팅방이 생성되었습니다.").c_str(), "채팅방 생성", MB_ICONINFORMATION);

    //// 새로 생성된 채팅방을 목록에 추가 (실제로는 서버에서 목록을 다시 받아야 함)
    //ChatRoomInfo newRoom = {
    //    (int)m_chatRooms.size() + 1,  // 임시 ID
    //    roomName,
    //    1,  // 자기 자신
    //    20  // 기본 최대 인원
    //};

    //m_chatRooms.push_back(newRoom);
    //UpdateLobby(m_chatRooms);
}

void LobbyWindow::UpdateLobby(const std::vector<ChatRoomInfo>& roomList)
{
    m_chatRooms = roomList;

    // 리스트박스 내용 초기화
    SendMessage(m_hRoomList, LB_RESETCONTENT, 0, 0);

    // 각 채팅방 정보를 리스트박스에 추가
    for (const auto& room : m_chatRooms)
    {
        std::wstring itemText = /*WinUtils::StringToWString(room.roomID) +*/ L" (" + std::to_wstring(room.currentUser) +
            L"/" + std::to_wstring(room.maxUser) + L")";
        SendMessage(m_hRoomList, LB_ADDSTRING, 0, (LPARAM)itemText.c_str());
    }
}

LRESULT LobbyWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 해당 윈도우의 ChatRoomList 객체 찾기
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
                // 선택된 채팅방 인덱스 가져오기
                int selectedIndex = SendMessage(pThis->m_hRoomList, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR && selectedIndex < pThis->m_chatRooms.size()) {
                    pThis->EnterChatRoom(pThis->m_chatRooms[selectedIndex].roomID);
                }
                else {
                    MessageBoxW(hwnd, L"채팅방을 선택해주세요.", L"알림", MB_ICONINFORMATION);
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_BUTTON_CREATE) {
                /// 새 채팅방 이름 가져오기
                int nameLength = GetWindowTextLengthW(pThis->m_hRoomNameEdit);
                if (nameLength > 0) {
                    std::vector<wchar_t> buffer(nameLength + 1);
                    GetWindowTextW(pThis->m_hRoomNameEdit, buffer.data(), nameLength + 1);
                    std::wstring wRoomName(buffer.data());

                    // 채팅방 이름을 UTF-8로 변환
                    std::string roomName = WinUtils::WStringToString(wRoomName);

                    // 새 채팅방 생성
                    pThis->CreateNewChatRoom(roomName);

                    // 입력 필드 초기화
                    SetWindowTextW(pThis->m_hRoomNameEdit, L"");
                }
                else {
                    MessageBoxW(hwnd, L"채팅방 이름을 입력해주세요.", L"알림", MB_ICONINFORMATION);
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
            // 채팅방 목록 응답 처리
            ChatRoomListResponseData* data = (ChatRoomListResponseData*)lParam;
            if (data) 
            {
                // 채팅방 목록 업데이트
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

                // 메모리 해제
                delete data;
            }
        }
        break;

    case WM_CLIENT_CHATROOM_ENTER:
        if (pThis) {
            ChatRoomResponseData* data = (ChatRoomResponseData*)lParam;
            if (data)
            {
                // 채팅방 목록 창 숨기기
                pThis->Hide();

                // 채팅창으로 전환 메시지 전송
                ::PostMessage(GetParent(hwnd), WM_ENTER_CHATROOM, 0, lParam);
                // lParam을 그대로 전달하므로 여기서는 메모리 해제하지 않음
            }
            else
            {
                //MessageBoxW(hwnd, WinUtils::StringToWString(data->message).c_str(), L"채팅방 입장 실패", MB_ICONERROR);
                // 실패한 경우에만 메모리 해제
                delete data;
            }
        }
        break;
    case WM_CLIENT_CHATROOM_CREATE:
        if (pThis)
        {
            ChatRoomResponseData* data = (ChatRoomResponseData*)lParam;
            if (data)
            {
                MessageBoxW(hwnd, WinUtils::StringToWString("채팅방 '" + data->roomName + "'이(가) 생성되었습니다.").c_str(), L"채팅방 생성 성공", MB_ICONINFORMATION);

                ChatRoomInfo newRoom = { data->roomId, 1, 10 };

                pThis->m_chatRooms.push_back(newRoom);
                pThis->UpdateLobby(pThis->m_chatRooms);
                
                // 목록 새로고침 요청
                pThis->RefreshRoomList();
            }
            else {
                //MessageBoxW(hwnd, WinUtils::StringToWString(data->message).c_str(), L"채팅방 생성 실패", MB_ICONERROR);
            }

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
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");

    // 환영 메시지 라벨
    m_hWelcomeLabel = CreateWindowW(L"STATIC", L"채팅방 목록",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        20, 20, 560, 25, m_hWnd, (HMENU)IDC_STATIC_WELCOME, NULL, NULL);
    SendMessage(m_hWelcomeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 채팅방 목록 리스트박스
    m_hRoomList = CreateWindowW(L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        20, 60, 560, 300, m_hWnd, (HMENU)IDC_LIST_ROOMS, NULL, NULL);
    SendMessage(m_hRoomList, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 채팅방 입장 버튼
    m_hEnterButton = CreateWindowW(L"BUTTON", L"입장",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        450, 370, 130, 30, m_hWnd, (HMENU)IDC_BUTTON_ENTER, NULL, NULL);
    SendMessage(m_hEnterButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 새 채팅방 이름 라벨
    HWND hStaticRoomName = CreateWindowW(L"STATIC", L"새 채팅방 이름:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 380, 130, 20, m_hWnd, (HMENU)IDC_STATIC_ROOM_NAME, NULL, NULL);
    SendMessage(hStaticRoomName, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 새 채팅방 이름 입력 필드
    m_hRoomNameEdit = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        150, 380, 200, 25, m_hWnd, (HMENU)IDC_EDIT_ROOM_NAME, NULL, NULL);
    SendMessage(m_hRoomNameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 새 채팅방 생성 버튼
    m_hCreateButton = CreateWindowW(L"BUTTON", L"생성",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 420, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_CREATE, NULL, NULL);
    SendMessage(m_hCreateButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 새로고침 버튼
    HWND hRefreshButton = CreateWindowW(L"BUTTON", L"새로고침",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 420, 100, 30, m_hWnd, (HMENU)IDC_BUTTON_REFRESH, NULL, NULL);
    SendMessage(hRefreshButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}
