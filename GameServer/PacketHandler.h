#pragma once
#include "Protocol.h"

class PacketHandler
{
public:
	using HandlerFunc = std::function<void(const Packet&)>;
	
	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	void HandlePacket(const Packet* packet);

private:
	std::unordered_map<PacketType, HandlerFunc> m_Handlers;
};

