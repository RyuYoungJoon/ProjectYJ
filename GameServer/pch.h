#pragma once
// ���⿡ �̸� �������Ϸ��� ��� �߰�
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.


#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif // _DEBUG

#pragma comment(lib, "Shlwapi.lib")

#include "CorePch.h"

using PlayerPtr = shared_ptr<class Player>;
using ChatRoomPtr = shared_ptr<class ChatRoom>;
using AsioSessionPtr = std::shared_ptr<class AsioSession>;
