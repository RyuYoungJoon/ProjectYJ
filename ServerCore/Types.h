#pragma once
#include <memory>

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned  __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

//using SessionPtr = std::shared_ptr<class AsioSession>;

using GameSessionPtr = std::shared_ptr<class GameSession>;
using AsioSessionPtr = std::shared_ptr<class AsioSession>;
using ClientSessionPtr = std::shared_ptr<class ClientSession>;
using ServerServicePtr = std::shared_ptr<class AsioServerService>;
using ClientServicePtr = std::shared_ptr<class AsioClientService>;