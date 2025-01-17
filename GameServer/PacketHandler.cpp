#include "pch.h"
#include "PacketHandler.h"

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
		std::cerr << "[ERROR] Unknown Packet Type : " << static_cast<int16>(packet->header.type) << std::endl;
	}
}

void PacketHandler::HandledefEchoString(const Packet& packet)
{
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));

	std::cout << "[SERVER INFO] PacketHandler::HandledefEchoString received: " << message << std::endl;

}

void PacketHandler::HandleJH(const Packet& packet)
{
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));

	std::cout << "[SERVER INFO] PacketHandler::HandleJH received: " << message << std::endl;

	// 추가 처리 로직
}

void PacketHandler::HandleYJ(const Packet& packet)
{
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));

	std::cout << "[SERVER INFO] PacketHandler::HandleYJ received: " << message << std::endl;

}

void PacketHandler::HandleES(const Packet& packet)
{
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));

	std::cout << "[SERVER INFO] PacketHandler::HandleES received: " << message << std::endl;

}
