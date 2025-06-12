#pragma once

// �α��� ���� ��� �ڵ�
enum LoginResult
{
	LOGIN_SUCCESS = 0,           // �α��� ����
	LOGIN_FAIL_INVALID_ID = 1,   // �߸��� ID
	LOGIN_FAIL_INVALID_PW = 2,   // �߸��� ��й�ȣ
	LOGIN_FAIL_ALREADY = 3,      // �̹� �α��ε� ����
	LOGIN_FAIL_SERVER = 4        // ���� ����
};

// ä�ù� ���� ��� �ڵ�
enum ChatRoomResult
{
	CHATROOM_SUCCESS = 0,          // ����
	CHATROOM_FAIL_NOT_EXIST = 1,   // �������� �ʴ� ä�ù�
	CHATROOM_FAIL_FULL = 2,        // ä�ù� ���� �ʰ�
	CHATROOM_FAIL_NO_PERMISSION = 3, // ���� ����
	CHATROOM_FAIL_SERVER = 4       // ���� ����
};

// ������ �޽��� ���� (LoginAck, ChatRoomListAck � ���� ó���� ����)
#define WM_CLIENT_LOGIN (WM_USER + 101)         // �α��� ����
#define WM_CLIENT_CHATROOM_LIST (WM_USER + 102) // ä�ù� ��� ����
#define WM_CLIENT_CHATROOM_ENTER (WM_USER + 103) // ä�ù� ���� ����
#define WM_CLIENT_CHATROOM_CREATE (WM_USER + 104) // ä�ù� ���� ����
#define WM_CLIENT_USER_LIST (WM_USER + 105)     // ����� ��� ����

// ���� �޽�����
#define WM_CLIENT_CONNECTED (WM_USER + 1)
#define WM_CLIENT_DISCONNECT (WM_USER + 2)
#define WM_CLIENT_RECV (WM_USER + 3)

// �α��� ���� �޽���
#define WM_CLIENT_LOGIN (WM_USER + 101)         // �α��� ����
#define WM_LOGIN_SUCCESS (WM_USER + 201)        // �α��� ���� (â ��ȯ��)

// ä�ù� ���� �޽���
#define WM_CLIENT_CHATROOM_LIST (WM_USER + 102) // ä�ù� ��� ����
#define WM_CLIENT_CHATROOM_ENTER (WM_USER + 103) // ä�ù� ���� ����
#define WM_CLIENT_CHATROOM_CREATE (WM_USER + 104) // ä�ù� ���� ����
#define WM_CLIENT_USER_LIST (WM_USER + 105)     // ����� ��� ����
#define WM_ENTER_CHATROOM (WM_USER + 202)       // ä�ù� ���� (â ��ȯ��)

#pragma pack(push, 1)
struct ChatRoomInfo
{
	uint16 roomID;
	//unsigned char roomName[32];
	uint16 currentUser;
	uint16 maxUser;
};

// �α��� ���� ������ ����ü
struct LoginResponseData
{
	LoginResult result;
	std::string userId;
	std::string message; // ���� �� �� �޽���
};

// ä�ù� ���� ����ü
struct ChatRoomData
{
	int roomId;
	std::string roomName;
	int currentUsers;
	int maxUsers;
};

// ä�ù� ��� ���� ������ ����ü
struct ChatRoomListResponseData
{
	std::vector<ChatRoomInfo> rooms;
};

// ä�ù� ����/���� ���� ������ ����ü
struct ChatRoomResponseData
{
	int roomId;
	std::string roomName;
};

// ����� ��� ���� ������ ����ü
struct UserListResponseData
{
	std::vector<std::string> users;
};
#pragma pack(pop)