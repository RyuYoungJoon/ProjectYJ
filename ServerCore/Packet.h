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
	uint8 checkSum[2];	// üũ��
	PacketType type;	// ��Ŷ Ÿ��
	uint32 size;		// ��Ŷ ��ü ������
	uint32 seqNum;		// ������ �ѹ�

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
	uint32 m_PayloadSize;	// ���� ������� ���̷ε� ������

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

	// ��Ŷ���
	PacketHeader& GetHeader() { return m_Header; }
	const PacketHeader& GetHeader() const { return m_Header; }

	// ���̷ε�
	uint8* GetPayload() { return m_Payload; }
	const uint8* GetPayload() const { return m_Payload; }

	uint32 GetPayloadSize() const { return m_PayloadSize; }
	uint32 GetMaxPayloadSize() const { return MAX_PAYLOAD_SIZE; }

	// ��Ŷ ������ ����
	bool SetPayload(const void* data, uint32 size);
	bool SetPayload(const std::string& data);

	// ��ü ��Ŷ ũ��
	uint32 GetTotalSize() const { return HEADER_SIZE + m_PayloadSize; }

	void Serialize(uint8* buffer);

	bool Deserialize(const uint8* buffer, uint32 totalSize);

	void SetCheckSum(uint8 byte1, uint8 byte2);

	void SetDefaultCheckSum();
};

