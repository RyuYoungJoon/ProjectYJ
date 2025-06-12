#pragma once

#define IDC_EDIT_MESSAGE 1001
#define IDC_BUTTON_SEND 1002
#define IDC_LIST_CHAT 1003
#define IDC_STATUS_BAR 1004

class ChatWindow
{
private:
	HWND m_hWnd;		
	HWND m_hEditMessage;
	HWND m_hSendButton;	
	HWND m_hListChat;	
	HWND m_hStatusBar;	

	std::mutex m_chatMutex;
	std::vector<std::wstring> m_chatMessage;
	bool m_isConnect;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam);

	static std::map<HWND, ChatWindow*> s_mapWindow;

	void CreateControl();

public:
	ChatWindow();
	~ChatWindow();

	bool Init(HINSTANCE hInstance);
	void Show();
	void Hide();
	bool IsVisible() const;
	void SetMainWnd(HWND hwnd) { m_hWnd = hwnd; }

	void OnConnect();
	void OnDisconnect();
	void OnMessageRecv(const std::string& sender, const std::string& message);
	void SendChatMessage();
	void AddChatMessage(const std::wstring& message);
	void UpdateStatus(bool isConnect);

	HWND GetHandle() const { return m_hWnd; }

	void SetTitle(const std::wstring& title);
};