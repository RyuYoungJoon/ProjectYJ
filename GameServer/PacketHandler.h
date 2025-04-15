#pragma once
#include "Protocol.h"
#include "PacketRouter.h"

class AsioSession;

class PacketHandler : public PacketProcessor
{
public:
	using HandlerFunc = std::function<void(AsioSessionPtr&, Packet*)>;
	
	static PacketHandler& GetInstance()
	{
		static PacketHandler instance;
		return instance;
	}
	
	PacketHandler();
	~PacketHandler();

	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	virtual void HandlePacket(AsioSessionPtr session, Packet* packet) override;
	
	void HandledefEchoString(AsioSessionPtr& session, Packet* packet);
	void HandleJH(AsioSessionPtr& session, Packet* packet);
	void HandleYJ(AsioSessionPtr& session, Packet* packet);
	void HandleES(AsioSessionPtr& session, Packet* packet);
	void HandleChatReq(AsioSessionPtr& session, Packet* packet);
	void HandleLoginReq(AsioSessionPtr& session, Packet* packet);

	void Reset(int32 sessionUID);

	static void HandleInvalid(AsioSessionPtr& session, Packet* packet);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<PacketType, std::atomic<int32>> m_RecvCount;
};