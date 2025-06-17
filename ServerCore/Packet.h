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
	// �⺻ ������
	Packet()
		: m_Data(nullptr), m_Size(0), m_PacketType(0), m_Buffer(nullptr){ }

	Packet(const char* data, uint32 size, uint32 packetType)
		: m_Size(size), m_PacketType(packetType)
	{
		m_Buffer = std::make_unique<char[]>(size);
		std::memcpy(m_Buffer.get(), data, size);
		m_Data = m_Buffer.get();
	}

	// �̵�
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

	// ���� ������
	Packet(const Packet& other)
		: m_Size(other.m_Size), m_PacketType(other.m_PacketType)
	{
		if (other.m_Data && other.m_Size > 0)
		{
			// ���ο� �޸� �Ҵ��ϰ� ������ ����
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

	// ���� ���� ������
	Packet& operator=(const Packet& other)
	{
		if (this != &other) // �ڱ� �ڽŰ��� ���� ����
		{
			m_Size = other.m_Size;
			m_PacketType = other.m_PacketType;

			if (other.m_Data && other.m_Size > 0)
			{
				// ���ο� �޸� �Ҵ��ϰ� ������ ����
				m_Buffer = std::make_unique<char[]>(other.m_Size);
				std::memcpy(m_Buffer.get(), other.m_Data, other.m_Size);
				m_Data = m_Buffer.get();
			}
			else
			{
				m_Data = nullptr;
				m_Buffer.reset(); // ���� �޸� ����
			}
		}
		return *this;
	}

	// Getter �Լ���
	const char* GetData() const { return m_Data; }
	uint32 GetSize() const { return m_Size; }
	uint32 GetPacketType() const { return m_PacketType; }
	bool IsValid() const { return m_Data != nullptr && m_Size > 0; }

	// �ܺ� ������ ���� (������ ����)
	void SetExternalData(const char* data, uint32 size, uint32 packetType)
	{
		m_Buffer.reset(); // ���� ���� ������ ����
		m_Data = data;
		m_Size = size;
		m_PacketType = packetType;
	}

	// ������
	void PrintInfo() const
	{
		LOGD << "Packet - Type: " << m_PacketType
			<< ", Size: " << m_Size
			<< ", Data: " << (m_Data ? "Valid" : "NULL");
	}
private:
	const char* m_Data;		// ������ ������
	uint32 m_Size;			// ������ ũ��
	uint32 m_PacketType;	// ��Ŷ Ÿ��
	std::unique_ptr<char[]> m_Buffer;	// ������ ������
};