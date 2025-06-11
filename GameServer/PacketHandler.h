#pragma once
#include "Packet.h"
#include "PacketRouter.h"
#include "Protocol.pb.h"
#include "Pool.h"

enum : uint16
{
	PKT_EnterChatRoomReq = 1001,
	PKT_EnterChatRoomAck = 1002,
};

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
	
	// 자동화 영역
	static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }

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

	template<typename T>
	static Packet MakePacket(T& pkt, uint16 packetType)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);
		
		PacketHeader header;
		header.packetType = packetType;
		header.packetSize = packetSize;
		header.seqNum = 1;

		auto buffer = std::make_unique<char>(header.packetSize);

		if (!pkt.SerializeToArray(buffer.get() + sizeof(PacketHeader), dataSize))
		{
			LOGE << "Faile SerializeToArray! PacketType : " << packetType;
			return Packet();
		}

		auto makePacket = PacketPool::GetInstance().Pop();

		Packet packet = Packet(buffer.release(), header.packetSize, packetType);

		return packet;
	}

private:
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<PacketType, std::atomic<int32>> m_RecvCount;
};