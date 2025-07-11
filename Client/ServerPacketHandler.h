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
	DummyPacketReq = 1012,
	DummyPacketAck = 1013,
};

class AsioSession;
bool HandleEnterChatRoomAck(AsioSessionPtr& session, Protocol::EnterChatRoomAck& pkt);
bool HandleLoginAck(AsioSessionPtr& session, Protocol::LoginAck& pkt);
bool HandleChatRoomListAck(AsioSessionPtr& session, Protocol::ChatRoomListAck& pkt);
bool HandleCreateChatRoomAck(AsioSessionPtr& session, Protocol::CreateChatRoomAck& pkt);
bool HandleRefreshChatRoomAck(AsioSessionPtr& session, Protocol::RefreshChatRoomAck& pkt);
bool HandleChatAck(AsioSessionPtr& session, Protocol::ChatAck& pkt);
bool HandleDummyPacketAck(AsioSessionPtr& session, Protocol::DummyPacketAck& pkt);

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
		GPacketHadler[EnterChatRoomAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::EnterChatRoomAck>(HandleEnterChatRoomAck, session, buffer, len); };
		GPacketHadler[LoginAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::LoginAck>(HandleLoginAck, session, buffer, len); };
		GPacketHadler[ChatRoomListAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::ChatRoomListAck>(HandleChatRoomListAck, session, buffer, len); };
		GPacketHadler[CreateChatRoomAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::CreateChatRoomAck>(HandleCreateChatRoomAck, session, buffer, len); };
		GPacketHadler[RefreshChatRoomAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::RefreshChatRoomAck>(HandleRefreshChatRoomAck, session, buffer, len); };
		GPacketHadler[ChatAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::ChatAck>(HandleChatAck, session, buffer, len); };
		GPacketHadler[DummyPacketAck] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::DummyPacketAck>(HandleDummyPacketAck, session, buffer, len); };
	}

	virtual bool HandlePacket(AsioSessionPtr& session, BYTE* buffer, int32 len) override
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHadler[header->packetType](session, buffer, len);
	}

	// 패킷 생성
	static Packet MakePacket(Protocol::EnterChatRoomReq& pkt) { return MakePacket(pkt, EnterChatRoomReq); }
	static Packet MakePacket(Protocol::LoginReq& pkt) { return MakePacket(pkt, LoginReq); }
	static Packet MakePacket(Protocol::ChatRoomListReq& pkt) { return MakePacket(pkt, ChatRoomListReq); }
	static Packet MakePacket(Protocol::CreateChatRoomReq& pkt) { return MakePacket(pkt, CreateChatRoomReq); }
	static Packet MakePacket(Protocol::RefreshChatRoomReq& pkt) { return MakePacket(pkt, RefreshChatRoomReq); }
	static Packet MakePacket(Protocol::ChatReq& pkt) { return MakePacket(pkt, ChatReq); }
	static Packet MakePacket(Protocol::DummyPacketReq& pkt) { return MakePacket(pkt, DummyPacketReq); }

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