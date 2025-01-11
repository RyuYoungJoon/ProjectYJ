#include "pch.h"
#include "PacketHandler.h"

void PacketHandler::RegisterHandler(PacketType packetType, HandlerFunc handler)
{
	auto iter = m_Handlers.find(packetType);
	if (iter == m_Handlers.end())
		m_Handlers.emplace(packetType, handler);
}

void PacketHandler::HandlePacket(const Packet& packet)
{
	auto iter = m_Handlers.find(packet.header.type);
	if (iter != m_Handlers.end())
	{
		iter->second(packet);
	}
	else
	{
		std::cerr << "[ERROR] Unknown Packet Type : " << static_cast<int16>(packet.header.type) << std::endl;
	}
}
