#pragma once
#include "Packet.h"
#include "PacketRouter.h"
#include "Protocol.pb.h"
#include "Pool.h"

using HandlerFunc = std::function<bool(AsioSessionPtr&, BYTE*, int32 len)>;
extern HandlerFunc GPacketHadler[UINT16_MAX];

enum : uint16
{
	EnterChatRoomReq = 1000,
	EnterChatRoomAck = 1001,
	LoginReq = 1002,
	LoginAck = 1003,
	ChatRoomListReq = 1004,
	ChatRoomListAck = 1005,
	CreateChatRoomReq = 1006,
	CreateChatRoomAck = 1007,
	RefreshChatRoomReq = 1008,
	RefreshChatRoomAck = 1009,
	ChatReq = 1010,
	ChatAck = 1011,
};

class AsioSession;
bool HandleEnterChatRoomReq(AsioSessionPtr& session, Protocol::EnterChatRoomReq& pkt);
bool HandleLoginReq(AsioSessionPtr& session, Protocol::LoginReq& pkt);
bool HandleChatRoomListReq(AsioSessionPtr& session, Protocol::ChatRoomListReq& pkt);
bool HandleCreateChatRoomReq(AsioSessionPtr& session, Protocol::CreateChatRoomReq& pkt);
bool HandleRefreshChatRoomReq(AsioSessionPtr& session, Protocol::RefreshChatRoomReq& pkt);
bool HandleChatReq(AsioSessionPtr& session, Protocol::ChatReq& pkt);

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
		GPacketHadler[EnterChatRoomReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::EnterChatRoomReq>(HandleEnterChatRoomReq, session, buffer, len); };
		GPacketHadler[LoginReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::LoginReq>(HandleLoginReq, session, buffer, len); };
		GPacketHadler[ChatRoomListReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::ChatRoomListReq>(HandleChatRoomListReq, session, buffer, len); };
		GPacketHadler[CreateChatRoomReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::CreateChatRoomReq>(HandleCreateChatRoomReq, session, buffer, len); };
		GPacketHadler[RefreshChatRoomReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::RefreshChatRoomReq>(HandleRefreshChatRoomReq, session, buffer, len); };
		GPacketHadler[ChatReq] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::ChatReq>(HandleChatReq, session, buffer, len); };
	}

	virtual bool HandlePacket(AsioSessionPtr& session, BYTE* buffer, int32 len) override
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHadler[header->packetType](session, buffer, len);
	}

	// 패킷 생성
	static Packet MakePacket(Protocol::EnterChatRoomAck& pkt) { return MakePacket(pkt, EnterChatRoomAck); }
	static Packet MakePacket(Protocol::LoginAck& pkt) { return MakePacket(pkt, LoginAck); }
	static Packet MakePacket(Protocol::ChatRoomListAck& pkt) { return MakePacket(pkt, ChatRoomListAck); }
	static Packet MakePacket(Protocol::CreateChatRoomAck& pkt) { return MakePacket(pkt, CreateChatRoomAck); }
	static Packet MakePacket(Protocol::RefreshChatRoomAck& pkt) { return MakePacket(pkt, RefreshChatRoomAck); }
	static Packet MakePacket(Protocol::ChatAck& pkt) { return MakePacket(pkt, ChatAck); }

	PacketHandler();
	~PacketHandler();

	bool RegisterHandler(uint16 packetType, HandlerFunc handler);
	void Reset(int32 sessionUID);
	void HandleInvalid(AsioSessionPtr& session, BYTE* buffer);

private:

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