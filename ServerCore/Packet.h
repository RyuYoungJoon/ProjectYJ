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
//
//enum class NetState : uint8
//{
//	None = 0,
//	Connecting = 1,
//	Connected = 2,
//	Disconnect = 3,
//	Reconnect = 4,
//};


class Packet
{
public:
	// 기본 생성자
	Packet()
		: m_Data(nullptr), m_Size(0), m_PacketType(0), m_Buffer(nullptr){ }

	Packet(unsigned char* data, uint32 size, uint32 packetType)
		: m_Size(size), m_PacketType(packetType)
	{
		m_Buffer = std::make_unique<unsigned char>(size);
		std::memcpy(m_Buffer.get(), data, size);
		m_Data = m_Buffer.get();
	}

	// 이동
	Packet(Packet&& other) noexcept
		: m_Data(other.m_Data), m_Size(other.m_Size),
		m_PacketType(other.m_PacketType), m_Buffer(std::move(other.m_Buffer))
	{
		other.m_Data = nullptr;
		other.m_Size = 0;
		other.m_PacketType = 0;
	}

	Packet& operator=(Packet&& other) noexcept
	{
		if (this != &other)
		{
			m_Data = other.m_Data;
			m_Size = other.m_Size;
			m_PacketType = other.m_PacketType;
			m_Buffer = std::move(other.m_Buffer);

			other.m_Data = nullptr;
			other.m_Size = 0;
			other.m_PacketType = 0;

		}

		return *this;
	}

	// 복사는 명시적 금지. (성능상이유)
	Packet(const Packet&) = delete;
	Packet& operator=(const Packet&) = delete;

	// Getter 함수들
	unsigned char* GetData() const { return m_Data; }
	uint32 GetSize() const { return m_Size; }
	uint32 GetPacketType() const { return m_PacketType; }
	bool IsValid() const { return m_Data != nullptr && m_Size > 0; }

	// 외부 데이터 참조 (소유권 없음)
	void SetExternalData(unsigned char* data, uint32 size, uint32 packetType)
	{
		m_Buffer.reset(); // 기존 소유 데이터 해제
		m_Data = data;
		m_Size = size;
		m_PacketType = packetType;
	}

	// 디버깅용
	void PrintInfo() const
	{
		LOGD << "Packet - Type: " << m_PacketType
			<< ", Size: " << m_Size
			<< ", Data: " << (m_Data ? "Valid" : "NULL");
	}
private:
	unsigned char* m_Data;		// 데이터 포인터
	uint32 m_Size;			// 데이터 크기
	uint32 m_PacketType;	// 패킷 타입
	std::unique_ptr<unsigned char> m_Buffer;	// 소유권 관리용
};