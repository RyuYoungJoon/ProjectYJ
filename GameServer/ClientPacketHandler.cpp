#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Logger.h"
#include "ServerAnalyzer.h"
#include "Player.h"
#include "ChatRoom.h"
#include "ChatRoomManager.h"

HandlerFunc GPacketHadler[UINT16_MAX];

atomic<int> a;
PacketHandler::PacketHandler()
    : PacketProcessor()
{
    Init();
}
PacketHandler::~PacketHandler()
{
}

bool PacketHandler::RegisterHandler(uint16 packetType, HandlerFunc handler)
{
	auto iter = m_Handlers.find(packetType);
	if (iter == m_Handlers.end())
		m_Handlers.emplace(packetType, handler);

    auto iter2 = m_RecvCount.find(packetType);
    if (iter2 == m_RecvCount.end())
        m_RecvCount.emplace(packetType, 0);

    return true;
}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_NextSeq.erase(sessionUID);
}

bool HandleEnterChatRoomReq(AsioSessionPtr& session, Protocol::EnterChatRoomReq& pkt)
{
    LOGI << "����!";
    return true;
}

bool HandleLoginReq(AsioSessionPtr& session, Protocol::LoginReq& pkt)
{
    return false;
}

bool HandleEnterChatRoomAck(AsioSessionPtr& session, Protocol::EnterChatRoomAck& pkt)
{
    return false;
}
