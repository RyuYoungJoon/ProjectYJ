#pragma once
//#include <Types.h>

enum class PacketType : uint8_t
{
	defEchoString = 100,
};

// 21 bytes
#pragma pack(push, 1)
struct PacketHeader {
	PacketType type;           // 기본 : 100
	uint8_t checkSum[16];
	uint32_t size;
};

// 1 byte
struct PacketTail {
	uint8_t value;
};

// 150 bytes
struct Packet {
	PacketHeader header;
	uint8_t		 payload[128];
	PacketTail	 tail;              // 기본 : 255
};
#pragma pack (pop)