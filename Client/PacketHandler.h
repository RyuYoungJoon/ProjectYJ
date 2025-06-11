#pragma once
//#include "Protocol.h"
#include "PacketRouter.h"
#include "Packet.h"
#include "..\GameServer\Protocol.pb.h"
#include "Pool.h"

enum : uint16
{
	PKT_EnterChatRoomReq = 1001,
	PKT_EnterChatRoomAck = 1002,
};
using HandlerFunc = std::function<bool(AsioSessionPtr&, BYTE*, int32 len)>;
extern HandlerFunc GPacketHadler[UINT16_MAX];

class AsioSession;

bool HandleRoomEnterAck(AsioSessionPtr& session, Protocol::EnterChatRoomAck& packet);

class PacketHandler : public PacketProcessor
{
public:

	static PacketHandler& GetInstance()
	{
		static PacketHandler instance;
		return instance;
	}

	//static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }
	static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }

	PacketHandler();
	~PacketHandler();

	void Init()
	{/*
		for (int32 i = 0; i < UINT16_MAX; ++i)
			GPacketHadler[i] = HandleInvalid;*/
		GPacketHadler[PKT_EnterChatRoomAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::EnterChatRoomAck>(HandleRoomEnterAck, session, buffer, len); };
	}

	bool RegisterHandler(uint16 packetType, HandlerFunc handler);
	
	virtual bool HandlePacket(AsioSessionPtr& session, BYTE* buffer, int32 len) override
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHadler[header->packetType](session, buffer, len);
	}

	void HandleLoginAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleChatAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomListAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomCreateAck(AsioSessionPtr& session, BYTE* buffer);

	void Reset(int32 sessionUID);

	static void HandleInvalid(AsioSessionPtr& session, BYTE* buffer);

public:
	
	template<typename PacketType, typename HandleFunc>
	static bool HandlePacket(HandleFunc func, AsioSessionPtr& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
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
		header.packetSize = sizeof(PacketHeader) + dataSize;
		header.seqNum = 1;

		auto buffer = std::make_unique<char>(header.packetSize);

		std::vector<char> buf(dataSize);
		if (!pkt.SerializeToArray(buf.data(), dataSize))
		{
			LOGE << "Faile SerializeToArray! PacketType : " << packetType;
			return Packet();
		}

		std::vector<char> packetBuffer(packetSize);

		memcpy(packetBuffer.data(), &header, sizeof(PacketHeader));

		memcpy(packetBuffer.data() + sizeof(PacketHeader), buf.data(), dataSize);
		auto makePacket = PacketPool::GetInstance().Pop();

		Packet packet = Packet(packetBuffer.data(), header.packetSize, packetType);

		return packet;
	}

private:
	std::map<uint16, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<uint16, std::atomic<int32>> m_RecvCount;
};