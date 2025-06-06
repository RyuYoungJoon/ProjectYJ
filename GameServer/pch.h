#pragma once
// 여기에 미리 컴파일하려는 헤더 추가
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.


#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif // _DEBUG

#pragma comment(lib, "Shlwapi.lib")

#include "CorePch.h"

using PlayerPtr = shared_ptr<class Player>;
using ChatRoomPtr = shared_ptr<class ChatRoom>;