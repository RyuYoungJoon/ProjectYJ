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
	// �⺻ ������
	Packet()
		: m_Data(nullptr), m_Size(0), m_PacketType(0), m_Buffer(nullptr){ }

	Packet(unsigned char* data, uint32 size, uint32 packetType)
		: m_Size(size), m_PacketType(packetType)
	{
		m_Buffer = std::make_unique<unsigned char>(size);
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

	// ����� ����� ����. (���ɻ�����)
	Packet(const Packet&) = delete;
	Packet& operator=(const Packet&) = delete;

	// Getter �Լ���
	unsigned char* GetData() const { return m_Data; }
	uint32 GetSize() const { return m_Size; }
	uint32 GetPacketType() const { return m_PacketType; }
	bool IsValid() const { return m_Data != nullptr && m_Size > 0; }

	// �ܺ� ������ ���� (������ ����)
	void SetExternalData(unsigned char* data, uint32 size, uint32 packetType)
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
	unsigned char* m_Data;		// ������ ������
	uint32 m_Size;			// ������ ũ��
	uint32 m_PacketType;	// ��Ŷ Ÿ��
	std::unique_ptr<unsigned char> m_Buffer;	// ������ ������
};