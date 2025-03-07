#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "Logger.h"

void PacketHandler::Init()
{
	RegisterHandler(PacketType::defEchoString, std::bind(&PacketHandler::HandledefEchoString, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::JH, std::bind(&PacketHandler::HandleJH, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::YJ, std::bind(&PacketHandler::HandleYJ, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::ES, std::bind(&PacketHandler::HandleES, this, std::placeholders::_1, std::placeholders::_2));
}

void PacketHandler::RegisterHandler(PacketType packetType, HandlerFunc handler)
{
	auto iter = m_Handlers.find(packetType);
	if (iter == m_Handlers.end())
		m_Handlers.emplace(packetType, handler);
}

void PacketHandler::HandlePacket(AsioSessionPtr& session, const Packet* packet)
{
	auto iter = m_Handlers.find(packet->header.type);
	if (iter != m_Handlers.end())
	{
		iter->second(session, *packet);
	}
	else
	{
		HandleInvalid(session, *packet);
	}
}

void PacketHandler::HandledefEchoString(AsioSessionPtr& session, const Packet& packet)
{
	if (packet.header.type != PacketType::defEchoString)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	LOGD << "SessionUID : "<<gameSession->GetSessionUID()<<", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
}

void PacketHandler::HandleJH(AsioSessionPtr& session, const Packet& packet)
{
	if (packet.header.type != PacketType::JH)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
	// 추가 처리 로직

}

void PacketHandler::HandleYJ(AsioSessionPtr& session, const Packet& packet)
{
	if (packet.header.type != PacketType::YJ)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
}

void PacketHandler::HandleES(AsioSessionPtr& session, const Packet& packet)
{
	if (packet.header.type != PacketType::ES)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
}

void PacketHandler::HandleInvalid(AsioSessionPtr& session, const Packet& packet)
{
	LOGE << "Unknown Packet Type : " << static_cast<int16>(packet.header.type);
}
