#pragma once

// 컨트롤 ID 정의
#define IDC_EDIT_MESSAGE 1001
#define IDC_BUTTON_SEND 1002
#define IDC_LIST_CHAT 1003
#define IDC_STATUS_BAR 1004

class ChatWindow
{
private:
	HWND m_hWnd;			// 메인 윈도우 핸들
	HWND m_hEditMessage;	// 메시지 입력 필드
	HWND m_hSendButton;		// 전송 버튼
	HWND m_hListChat;		// 채팅 기록 리스트 박스
	HWND m_hStatusBar;		// 상태바

	// 채팅 데이터
	std::mutex m_chatMutex;
	std::vector<std::wstring> m_chatMessage;
	bool m_isConnect;

	// 윈도우 프로시저 (static으로 선언해야 WinApi에서 사용 가능)
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam);

	// 윈도우 핸들과 객체 연결을 위한 맵
	static std::map<HWND, ChatWindow*> s_mapWindow;

	// 컨트롤 생성
	void CreateControl();

public:
	ChatWindow();
	~ChatWindow();

	// 초기화 및 창 표시
	bool Init(HINSTANCE hInstance);
	void Show();
	void Hide();
	bool IsVisible() const;
	void SetMainWnd(HWND hwnd) { m_hWnd = hwnd; }

	// 이벤트 처리
	void OnConnect();
	void OnDisconnect();
	void OnMessageRecv(const std::string& sender, const std::string& message);
	void SendChatMessage();
	void AddChatMessage(const std::wstring& message);
	void UpdateStatus(bool isConnect);

	// 창 핸들 열기
	HWND GetHandle() const { return m_hWnd; }

	// 창 설정
	void SetTitle(const std::wstring& title);
};