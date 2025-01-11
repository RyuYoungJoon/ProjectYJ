#pragma once
#include <Types.h>

enum class PacketType : uint8
{
	defEchoString = 100,
};

// 21 bytes
#pragma pack(push, 1)
struct PacketHeader {
	PacketType type;           // 기본 : 100
	BYTE checkSum[16];
	uint32 size;
};

// 1 byte
struct PacketTail {
	uint8 value;
};

// 150 bytes
struct Packet {
	PacketHeader header;
	BYTE		 payload[128];
	PacketTail	 tail;              // 기본 : 255
};
#pragma pack (pop)