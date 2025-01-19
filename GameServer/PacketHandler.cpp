#include "pch.h"
#include "PacketHandler.h"
#include "Logger.h"

void PacketHandler::Init()
{
	RegisterHandler(PacketType::defEchoString, std::bind(&PacketHandler::HandledefEchoString, this, std::placeholders::_1));
	RegisterHandler(PacketType::JH, std::bind(&PacketHandler::HandleJH, this, std::placeholders::_1));
	RegisterHandler(PacketType::YJ, std::bind(&PacketHandler::HandleYJ, this, std::placeholders::_1));
	RegisterHandler(PacketType::ES, std::bind(&PacketHandler::HandleES, this, std::placeholders::_1));
}

void PacketHandler::RegisterHandler(PacketType packetType, HandlerFunc handler)
{
	auto iter = m_Handlers.find(packetType);
	if (iter == m_Handlers.end())
		m_Handlers.emplace(packetType, handler);
}

void PacketHandler::HandlePacket(const Packet* packet)
{
	auto iter = m_Handlers.find(packet->header.type);
	if (iter != m_Handlers.end())
	{
		iter->second(*packet);
	}
	else
	{
		LOGE << "Unknown Packet Type : " << static_cast<int16>(packet->header.type);
	}
}

void PacketHandler::HandledefEchoString(const Packet& packet)
{
	if (packet.header.type != PacketType::defEchoString)
		return;

	LOGD << packet.payload;
}

void PacketHandler::HandleJH(const Packet& packet)
{
	if (packet.header.type != PacketType::JH)
		return;

	LOGD << packet.payload;
	// 추가 처리 로직
}

void PacketHandler::HandleYJ(const Packet& packet)
{
	if (packet.header.type != PacketType::YJ)
		return;

	LOGD << packet.payload;
}

void PacketHandler::HandleES(const Packet& packet)
{
	if (packet.header.type != PacketType::ES)
		return;

	LOGD << packet.payload;
}
