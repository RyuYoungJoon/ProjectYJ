#pragma once

// 로그인 응답 결과 코드
enum LoginResult
{
	LOGIN_SUCCESS = 0,           // 로그인 성공
	LOGIN_FAIL_INVALID_ID = 1,   // 잘못된 ID
	LOGIN_FAIL_INVALID_PW = 2,   // 잘못된 비밀번호
	LOGIN_FAIL_ALREADY = 3,      // 이미 로그인된 계정
	LOGIN_FAIL_SERVER = 4        // 서버 오류
};

// 채팅방 응답 결과 코드
enum ChatRoomResult
{
	CHATROOM_SUCCESS = 0,          // 성공
	CHATROOM_FAIL_NOT_EXIST = 1,   // 존재하지 않는 채팅방
	CHATROOM_FAIL_FULL = 2,        // 채팅방 정원 초과
	CHATROOM_FAIL_NO_PERMISSION = 3, // 권한 없음
	CHATROOM_FAIL_SERVER = 4       // 서버 오류
};

// 윈도우 메시지 정의 (LoginAck, ChatRoomListAck 등에 대한 처리를 위해)
#define WM_CLIENT_LOGIN (WM_USER + 101)         // 로그인 응답
#define WM_CLIENT_CHATROOM_LIST (WM_USER + 102) // 채팅방 목록 응답
#define WM_CLIENT_CHATROOM_ENTER (WM_USER + 103) // 채팅방 입장 응답
#define WM_CLIENT_CHATROOM_CREATE (WM_USER + 104) // 채팅방 생성 응답
#define WM_CLIENT_USER_LIST (WM_USER + 105)     // 사용자 목록 응답

// 기존 메시지들
#define WM_CLIENT_CONNECTED (WM_USER + 1)
#define WM_CLIENT_DISCONNECT (WM_USER + 2)
#define WM_CLIENT_RECV (WM_USER + 3)

// 로그인 관련 메시지
#define WM_CLIENT_LOGIN (WM_USER + 101)         // 로그인 응답
#define WM_LOGIN_SUCCESS (WM_USER + 201)        // 로그인 성공 (창 전환용)

// 채팅방 관련 메시지
#define WM_CLIENT_CHATROOM_LIST (WM_USER + 102) // 채팅방 목록 응답
#define WM_CLIENT_CHATROOM_ENTER (WM_USER + 103) // 채팅방 입장 응답
#define WM_CLIENT_CHATROOM_CREATE (WM_USER + 104) // 채팅방 생성 응답
#define WM_CLIENT_USER_LIST (WM_USER + 105)     // 사용자 목록 응답
#define WM_ENTER_CHATROOM (WM_USER + 202)       // 채팅방 입장 (창 전환용)

// 로그인 응답 데이터 구조체
struct LoginResponseData
{
	LoginResult result;
	std::string userId;
	std::string message; // 실패 시 상세 메시지
};

// 채팅방 정보 구조체
struct ChatRoomData
{
	int roomId;
	std::string roomName;
	int currentUsers;
	int maxUsers;
};

// 채팅방 목록 응답 데이터 구조체
struct ChatRoomListResponseData
{
	std::vector<ChatRoomData> rooms;
};

// 채팅방 입장/생성 응답 데이터 구조체
struct ChatRoomResponseData
{
	ChatRoomResult result;
	int roomId;
	std::string roomName;
	std::string message; // 실패 시 상세 메시지
};

// 사용자 목록 응답 데이터 구조체
struct UserListResponseData
{
	std::vector<std::string> users;
};