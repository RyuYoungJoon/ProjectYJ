#pragma once
// ���⿡ �̸� �������Ϸ��� ��� �߰�
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.


#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif // _DEBUG

#pragma comment(lib, "Shlwapi.lib")

#include "CorePch.h"

using PlayerPtr = shared_ptr<class Player>;
using ChatRoomPtr = shared_ptr<class ChatRoom>;