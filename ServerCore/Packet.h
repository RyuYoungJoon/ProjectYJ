#pragma once
enum class PacketType : uint8
{
	None = 0,
	StressTestPacket = 100,
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

enum class NetState : uint8
{
	None = 0,
	Connecting = 1,
	Connected = 2,
	Disconnected = 3,
};

struct PacketHeader
{
	uint8 checkSum[2];	// 체크섬
	PacketType type;	// 패킷 타입
	uint32 size;		// 패킷 전체 사이즈
	uint32 seqNum;		// 시퀀스 넘버

	PacketHeader()
	{
		memset(checkSum, 0, sizeof(checkSum));
		type = PacketType::None;
		size = 0;
		seqNum = 0;
	}
};

class Packet
{
public:
	static constexpr uint32 MAX_PAYLOAD_SIZE = 4096;
	static constexpr uint32 HEADER_SIZE = sizeof(PacketHeader);
	static constexpr uint32 MAX_PACKET_SIZE = HEADER_SIZE + MAX_PAYLOAD_SIZE;

private:
	PacketHeader m_Header;
	uint8 m_Payload[MAX_PAYLOAD_SIZE];
	uint32 m_PayloadSize;	// 실제 사용중인 페이로드 사이즈

public:
	Packet()
	{
		Reset();
	}

	void Reset()
	{
		m_Header = PacketHeader();
		memset(m_Payload, 0, sizeof(m_Payload));
		m_PayloadSize = 0;
	}

	// 패킷헤더
	PacketHeader& GetHeader() { return m_Header; }
	const PacketHeader& GetHeader() const { return m_Header; }

	// 페이로드
	uint8* GetPayload() { return m_Payload; }
	const uint8* GetPayload() const { return m_Payload; }

	uint32 GetPayloadSize() const { return m_PayloadSize; }
	uint32 GetMaxPayloadSize() const { return MAX_PAYLOAD_SIZE; }

	// 패킷 데이터 설정
	bool SetPayload(const void* data, uint32 size);
	bool SetPayload(const std::string& data);

	// 전체 패킷 크기
	uint32 GetTotalSize() const { return HEADER_SIZE + m_PayloadSize; }

	void Serialize(uint8* buffer);

	bool Deserialize(const uint8* buffer, uint32 totalSize);

	void SetCheckSum(uint8 byte1, uint8 byte2);

	void SetDefaultCheckSum();
};

