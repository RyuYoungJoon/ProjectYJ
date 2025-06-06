#pragma once
#include "Protocol.h"
#include "PacketRouter.h"

class AsioSession;

class PacketHandler : public PacketProcessor
{
public:
	using HandlerFunc = std::function<void(AsioSessionPtr&, BYTE*)>;
	
	static PacketHandler& GetInstance()

	{
		static PacketHandler instance;
		return instance;
	}
	
	PacketHandler();
	~PacketHandler();

	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	virtual void HandlePacket(AsioSessionPtr session, BYTE* packet) override;
	
	void HandleDummyClient(AsioSessionPtr& session, BYTE* buffer);
	void HandleChatReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleLoginReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomEnterReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomCreateReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomListReq(AsioSessionPtr& session, BYTE* buffer);

	void Reset(int32 sessionUID);

	static void HandleInvalid(AsioSessionPtr& session, BYTE* buffer);

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<PacketType, std::atomic<int32>> m_RecvCount;
};