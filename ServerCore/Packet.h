#pragma once
//enum class PacketType : uint8
//{
//	None = 0,
//	StressTestPacket = 100,
//	ChatReq = 104,
//	ChatAck = 105,
//	LoginReq = 106,
//	LoginAck = 107,
//	RoomListReq = 108,
//	RoomListAck = 109,
//	RoomEnterReq = 110,
//	RoomEnterAck = 111,
//	RoomCreateReq = 112,
//	RoomCreateAck = 113,
//	UserListReq = 114,
//	UserListAck = 115,
//};
//
//enum class NetState : uint8
//{
//	None = 0,
//	Connecting = 1,
//	Connected = 2,
//	Disconnect = 3,
//	Reconnect = 4,
//};

struct PacketHeader
{
	uint16 packetType;
	uint32 packetSize;
	uint32 seqNum;

	PacketHeader()
	{
		packetType = 0;
		packetSize = 0;
		seqNum = 0;
	}
};

class Packet
{
public:
	// 기본 생성자
	Packet()
		: m_Data(nullptr), m_Size(0), m_PacketType(0), m_Buffer(nullptr){ }

	Packet(const char* data, uint32 size, uint32 packetType)
		: m_Size(size), m_PacketType(packetType)
	{
		m_Buffer = std::make_unique<char[]>(size);
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

	// 복사 생성자
	Packet(const Packet& other)
		: m_Size(other.m_Size), m_PacketType(other.m_PacketType)
	{
		if (other.m_Data && other.m_Size > 0)
		{
			// 새로운 메모리 할당하고 데이터 복사
			m_Buffer = std::make_unique<char[]>(other.m_Size);
			std::memcpy(m_Buffer.get(), other.m_Data, other.m_Size);
			m_Data = m_Buffer.get();
		}
		else
		{
			m_Data = nullptr;
			m_Buffer = nullptr;
		}
	}

	// 복사 대입 연산자
	Packet& operator=(const Packet& other)
	{
		if (this != &other) // 자기 자신과의 대입 방지
		{
			m_Size = other.m_Size;
			m_PacketType = other.m_PacketType;

			if (other.m_Data && other.m_Size > 0)
			{
				// 새로운 메모리 할당하고 데이터 복사
				m_Buffer = std::make_unique<char[]>(other.m_Size);
				std::memcpy(m_Buffer.get(), other.m_Data, other.m_Size);
				m_Data = m_Buffer.get();
			}
			else
			{
				m_Data = nullptr;
				m_Buffer.reset(); // 기존 메모리 해제
			}
		}
		return *this;
	}

	// Getter 함수들
	const char* GetData() const { return m_Data; }
	uint32 GetSize() const { return m_Size; }
	uint32 GetPacketType() const { return m_PacketType; }
	bool IsValid() const { return m_Data != nullptr && m_Size > 0; }

	// 외부 데이터 참조 (소유권 없음)
	void SetExternalData(const char* data, uint32 size, uint32 packetType)
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
	const char* m_Data;		// 데이터 포인터
	uint32 m_Size;			// 데이터 크기
	uint32 m_PacketType;	// 패킷 타입
	std::unique_ptr<char[]> m_Buffer;	// 소유권 관리용
};