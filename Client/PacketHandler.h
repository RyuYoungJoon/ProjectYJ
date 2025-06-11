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

	//static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }
	static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, PKT_EnterChatRoomReq); }

	PacketHandler();
	~PacketHandler();

	void Init();

	void RegisterHandler(PacketType packetType, HandlerFunc handler);

	virtual void HandlePacket(AsioSessionPtr session, BYTE* buffer) override;

	void HandleLoginAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleChatAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomEnterAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomListAck(AsioSessionPtr& session, BYTE* buffer);
	void HandleRoomCreateAck(AsioSessionPtr& session, BYTE* buffer);

	void Reset(int32 sessionUID);

	static void HandleInvalid(AsioSessionPtr& session, BYTE* buffer);

public:

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
	std::map<PacketType, HandlerFunc> m_Handlers;
	std::map<int32, int32> m_NextSeq;
	std::mutex m_Mutex;
	std::map<PacketType, std::atomic<int32>> m_RecvCount;
};