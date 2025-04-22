#pragma once
#include "enum.h"

enum class PacketType : uint8_t
{
	defEchoString = 100,
	JH = 101,
	YJ = 102,
	ES = 103,
	ChatReq = 104,
	ChatAck = 105,
	LoginReq = 106,
	LoginAck = 107,
	RoomListReq = 108,
	RoomListAck = 109,
	RoomEnterReq = 110,
	RoomEnterAck = 111,
	RoomCreateReq = 112,
	RoomCreateAck = 113,
	UserListReq = 114,
	UserListAck = 115,
};

// 21 bytes
#pragma pack(push, 1)
struct PacketHeader {
	uint32_t seqNum;
	PacketType type;           // 기본 : 100
	uint8_t checkSum[16];
	uint32_t size;
};

// 1 byte
struct PacketTail {
	uint8 value;
};

template<PacketType Type, typename TPayload>
struct Packet
{
	PacketHeader header;
	TPayload payload;
	PacketTail tail;

	Packet()
	{
		memset(&header, 0, sizeof(header));
		memset(&payload, 0, sizeof(payload));
		header.type = Type;
		header.size = sizeof(TPayload);
		tail.value = 255;
	}
};
// 패킷
struct LoginPayloadReq
{
	std::string id;
	std::string password;
};

struct LoginPayloadAck
{
	std::string id;
};

struct ChatPayloadReq
{
	std::string message;
};

struct ChatPayloadAck
{
	std::string sender;
	std::string message;
};

struct RoomEnterPayloadReq
{
	uint16 roomID;
};

struct RoomEnterPayloadAck
{
	uint16 roomID;
	std::string roomName;
	std::string message;
};

struct RoomCreatePayloadReq
{

};

struct RoomCreatePayloadAck
{

};

struct RoomListPayloadReq
{
	
};

struct RoomListPayloadAck
{
	std::vector<ChatRoomInfo> chatRoomInfo;
};

using PacketLoginReq = Packet<PacketType::LoginReq, LoginPayloadReq>;
using PacketLoginAck = Packet<PacketType::LoginAck, LoginPayloadAck>;
using PacketChatReq = Packet<PacketType::ChatReq, ChatPayloadReq>;
using PacketChatAck = Packet<PacketType::ChatAck, ChatPayloadAck>;
using PacketRoomEnterReq = Packet<PacketType::RoomEnterReq, RoomEnterPayloadReq>;
using PacketRoomEnterAck = Packet<PacketType::RoomEnterAck, RoomEnterPayloadAck>;
using PacketRoomCreateReq = Packet<PacketType::RoomCreateReq, RoomCreatePayloadReq>;
using PacketRoomCreateAck = Packet<PacketType::RoomCreateAck, RoomCreatePayloadAck>;
using PacketRoomListReq = Packet<PacketType::RoomListReq, RoomListPayloadReq>;
using PacketRoomListAck = Packet<PacketType::RoomListAck, RoomListPayloadAck>;
#pragma pack (pop)