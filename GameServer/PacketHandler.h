#pragma once
#include "Packet.h"
#include "PacketRouter.h"
#include "Protocol.pb.h"
#include "Pool.h"

using HandlerFunc = std::function<bool(AsioSessionPtr&, BYTE*, int32 len)>;
extern HandlerFunc GPacketHadler[UINT16_MAX];

enum : uint16
{
	PKT_EnterChatRoomReq = 1001,
	PKT_EnterChatRoomAck = 1002,
};

class AsioSession;

bool HandleRoomEnterReq(AsioSessionPtr& session, Protocol::EnterChatRoomReq& pkt);

class PacketHandler : public PacketProcessor
{
public:
	
	static PacketHandler& GetInstance()

	{
		static PacketHandler instance;
		return instance;
	}
	
	void Init()
	{/*
		for (int32 i = 0; i < UINT16_MAX; ++i)
			GPacketHadler[i] = HandleInvalid;*/
		GPacketHadler[PKT_EnterChatRoomReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::EnterChatRoomReq>(HandleRoomEnterReq, session, buffer, len); };
	}

	virtual bool HandlePacket(AsioSessionPtr& session, BYTE* buffer, int32 len) override
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHadler[header->packetType](session, buffer, len);
	}

	// 자동화 영역
	static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }

	PacketHandler();
	~PacketHandler();

	bool RegisterHandler(uint16 packetType, HandlerFunc handler);
	
	void HandleDummyClient(AsioSessionPtr& session, BYTE* buffer);
	void HandleChatReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleLoginReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomCreateReq(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomListReq(AsioSessionPtr& session, BYTE* buffer);

	void Reset(int32 sessionUID);

	void HandleInvalid(AsioSessionPtr& session, BYTE* buffer);

private:
	
	template<typename PacketType, typename HandleFunc>
	static bool HandlePacket(HandleFunc func, AsioSessionPtr& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		int32 packetsize = sizeof(PacketHeader);
		int temp = len - sizeof(PacketHeader);

		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

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
	std::map<uint16, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<uint16, std::atomic<int32>> m_RecvCount;
};