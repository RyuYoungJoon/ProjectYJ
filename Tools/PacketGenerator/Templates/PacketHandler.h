#pragma once
#include "Packet.h"
#include "PacketRouter.h"
#include "Protocol.pb.h"
#include "Pool.h"

using HandlerFunc = std::function<bool(AsioSessionPtr&, BYTE*, int32 len)>;
extern HandlerFunc GPacketHadler[UINT16_MAX];

enum : uint16
{
{%- for packet in parser.total_packet %}
	{{packet.name}} = {{packet.type}},
{%- endfor%}
};

class AsioSession;

{%- for packet in parser.ack_packet %}
bool Handle{{packet.name}}(AsioSessionPtr& session, Protocol::{{packet.name}}& pkt);
{%- endfor%}

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
{%- for packet in parser.ack_packet %}
		GPacketHadler[{{packet.name}}] = [](AsioSessionPtr& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::{{packet.name}}>(Handle{{packet.name}}, session, buffer, len); };
{%- endfor%}
	}

	virtual bool HandlePacket(AsioSessionPtr& session, BYTE* buffer, int32 len) override
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHadler[header->packetType](session, buffer, len);
	}

	// 패킷 생성
{%- for packet in parser.request_packet %}
	static Packet MakePacket(Protocol::{{packet.name}}& pkt) { return MakePacket(pkt, {{packet.name}}); }
{%- endfor%}

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