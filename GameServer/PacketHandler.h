#pragma once
#include "Protocol.h"

class AsioSession;

class PacketHandler
{
public:
	using HandlerFunc = std::function<void(shared_ptr<AsioSession>&, const Packet&)>;
	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	void HandlePacket(shared_ptr<AsioSession>& session, const Packet* packet);
	
	void HandledefEchoString(shared_ptr<AsioSession>& session, const Packet& packet);
	void HandleJH(shared_ptr<AsioSession>& session, const Packet& packet);
	void HandleYJ(shared_ptr<AsioSession>& session, const Packet& packet);
	void HandleES(shared_ptr<AsioSession>& session, const Packet& packet);

	static void HandleInvalid(shared_ptr<AsioSession>& session, const Packet& packet);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
};

