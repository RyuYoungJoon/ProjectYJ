#pragma once
#include "Protocol.h"

class AsioSession;

class PacketHandler
{
public:
	using HandlerFunc = std::function<void(AsioSessionPtr&, const Packet*)>;
	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	void HandlePacket(AsioSessionPtr& session, const Packet* packet);
	
	void HandledefEchoString(AsioSessionPtr& session, const Packet* packet);
	void HandleJH(AsioSessionPtr& session, const Packet* packet);
	void HandleYJ(AsioSessionPtr& session, const Packet* packet);
	void HandleES(AsioSessionPtr& session, const Packet* packet);

	void ProcessPendingPacket(AsioSessionPtr& session, int32 sessionUID);
	void Reset(int32 sessionUID);

	static void HandleInvalid(AsioSessionPtr& session, const Packet* packet);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<int32, std::queue<const Packet*>> m_PendingPacket;
};