#include "pch.h"
#include "Packet.h"

bool Packet::SetPayload(const void* data, uint32 size)
{
	if (size > MAX_PAYLOAD_SIZE)
		return false;

	memcpy(m_Payload, data, size);
	m_PayloadSize = size;
	m_Header.size = HEADER_SIZE + size;

	return true;
}

bool Packet::SetPayload(const std::string& data)
{
	return SetPayload(data.c_str(), static_cast<uint32>(data.length()));
}

void Packet::Serialize(uint8* buffer)
{
	uint32 offset = 0;

	// 헤더 복사
	memcpy(buffer + offset, &m_Header, HEADER_SIZE);
	offset += HEADER_SIZE;

	// 페이로드 복사
	if (m_PayloadSize > 0)
	{
		memcpy(buffer + offset, m_Payload, m_PayloadSize);
		offset += m_PayloadSize;
	}
}

bool Packet::Deserialize(const uint8* buffer, uint32 totalSize)
{
	if (totalSize < HEADER_SIZE)
		return false;

	uint32 offset = 0;

	// 헤더 복사
	memcpy(&m_Header, buffer + offset, HEADER_SIZE);
	offset += HEADER_SIZE;

	// 페이로드 크기 계산
	m_PayloadSize = totalSize - HEADER_SIZE;
	if (m_PayloadSize > 0)
	{
		memcpy(m_Payload, buffer + offset, m_PayloadSize);
		offset += m_PayloadSize;
	}

	return true;
}

void Packet::SetCheckSum(uint8 byte1, uint8 byte2)
{
	m_Header.checkSum[0] = byte1;
	m_Header.checkSum[1] = byte2;
}

void Packet::SetDefaultCheckSum()
{
	SetCheckSum(0x12, 0x34);
}

inline std::shared_ptr<Packet> CreatePacket()
{
	return std::make_shared<Packet>();
}

namespace PacketHelper
{
	inline Packet CreatePacket(PacketType type, const void* data, uint32 size, uint32 seqNum = 0)
	{
		Packet packet;
		packet.GetHeader().type = type;
		packet.GetHeader().seqNum = seqNum;
		packet.SetDefaultCheckSum();
		packet.SetPayload(data, size);

		return packet;
	}
}