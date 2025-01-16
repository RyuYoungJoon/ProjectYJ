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
		std::cerr << "[ERROR] Unknown Packet Type : " << static_cast<int16>(packet->header.type) << std::endl;
	}
}

void PacketHandler::HandledefEchoString(const Packet& packet)
{
	std::string InfoMessage("[SERVER INFO] PacketHandler::HandledefEchoString");
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
	
	cout << Logger::DLog(InfoMessage + message) << endl;

}

void PacketHandler::HandleJH(const Packet& packet)
{
	std::string InfoMessage("[SERVER INFO] PacketHandler::HandleJH");
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
	
	cout << Logger::DLog(InfoMessage + message) << endl;

	// 추가 처리 로직
}

void PacketHandler::HandleYJ(const Packet& packet)
{
	std::string InfoMessage("[SERVER INFO] PacketHandler::HandleYJ");
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));
	
	cout << Logger::DLog(InfoMessage + message) << endl;

	//std::cout << "[SERVER INFO] PacketHandler::HandleYJ received: " << message << std::endl;

}

void PacketHandler::HandleES(const Packet& packet)
{
	std::string InfoMessage("[SERVER INFO] PacketHandler::HandleES");
	std::string message(packet.payload, packet.payload + packet.header.size - sizeof(PacketHeader));

	cout << Logger::DLog(InfoMessage + message) << endl;

}
