#pragma once
//#include <Types.h>

enum class PacketType : uint8_t
{
	defEchoString = 100,
	JH = 101,
	YJ = 102,
	ES = 103,
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
	uint8 value;
};

// 150 bytes
struct Packet {
	PacketHeader header;
	unsigned char		 payload[128]{};
	PacketTail	 tail;              // 기본 : 255
};
#pragma pack (pop)