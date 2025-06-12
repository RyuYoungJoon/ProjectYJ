#pragma once

// ��Ʈ�� ID ����
#define IDC_EDIT_MESSAGE 1001
#define IDC_BUTTON_SEND 1002
#define IDC_LIST_CHAT 1003
#define IDC_STATUS_BAR 1004

class ChatWindow
{
private:
	HWND m_hWnd;			// ���� ������ �ڵ�
	HWND m_hEditMessage;	// �޽��� �Է� �ʵ�
	HWND m_hSendButton;		// ���� ��ư
	HWND m_hListChat;		// ä�� ��� ����Ʈ �ڽ�
	HWND m_hStatusBar;		// ���¹�

	// ä�� ������
	std::mutex m_chatMutex;
	std::vector<std::wstring> m_chatMessage;
	bool m_isConnect;

	// ������ ���ν��� (static���� �����ؾ� WinApi���� ��� ����)
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam);

	// ������ �ڵ�� ��ü ������ ���� ��
	static std::map<HWND, ChatWindow*> s_mapWindow;

	// ��Ʈ�� ����
	void CreateControl();

public:
	ChatWindow();
	~ChatWindow();

	// �ʱ�ȭ �� â ǥ��
	bool Init(HINSTANCE hInstance);
	void Show();
	void Hide();
	bool IsVisible() const;
	void SetMainWnd(HWND hwnd) { m_hWnd = hwnd; }

	// �̺�Ʈ ó��
	void OnConnect();
	void OnDisconnect();
	void OnMessageRecv(const std::string& sender, const std::string& message);
	void SendChatMessage();
	void AddChatMessage(const std::wstring& message);
	void UpdateStatus(bool isConnect);

	// â �ڵ� ����
	HWND GetHandle() const { return m_hWnd; }

	// â ����
	void SetTitle(const std::wstring& title);
};