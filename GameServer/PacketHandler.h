#pragma once
#include "Protocol.h"

class PacketHandler
{
public:
	using HandlerFunc = std::function<void(const Packet&)>;
	
	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	void HandlePacket(const Packet* packet);
	
	void HandledefEchoString(const Packet& packet);
	void HandleJH(const Packet& packet);
	void HandleYJ(const Packet& packet);
	void HandleES(const Packet& packet);

	static void HandleInvalid(const Packet& packet);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
};

