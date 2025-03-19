#pragma once
#include "Protocol.h"
#include "PacketRouter.h"

class AsioSession;

class PacketHandler : public PacketProcessor
{
public:
	using HandlerFunc = std::function<void(AsioSessionPtr&, const Packet*)>;
	
	static PacketHandler& GetInstance()
	{
		static PacketHandler instance;
		return instance;
	}
	
	PacketHandler();
	~PacketHandler();

	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	virtual void HandlePacket(AsioSessionPtr session, const Packet* packet) override;
	
	void HandledefEchoString(AsioSessionPtr& session, const Packet* packet);
	void HandleJH(AsioSessionPtr& session, const Packet* packet);
	void HandleYJ(AsioSessionPtr& session, const Packet* packet);
	void HandleES(AsioSessionPtr& session, const Packet* packet);

	void ProcessPendingPacket(AsioSessionPtr& session, int32 sessionUID);
	void Reset(int32 sessionUID);

	virtual void Test() override;

	static void HandleInvalid(AsioSessionPtr& session, const Packet* packet);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<int32, std::queue<const Packet*>> m_PendingPacket;
};