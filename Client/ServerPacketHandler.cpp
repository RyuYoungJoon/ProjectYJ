#include "pch.h"
#include "ServerPacketHandler.h"
#include "ClientSession.h"

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


void PacketHandler::HandleInvalid(AsioSessionPtr& session, BYTE* buffer)
{
   // PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

  //  LOGE << "Unknown Packet Type : " << static_cast<int16>(header->type);
}

bool HandleRoomEnterAck(AsioSessionPtr& session, Protocol::EnterChatRoomAck& packet)
{
    LOGI << "����!";

    return true;
}

bool HandleEnterChatRoomReq(AsioSessionPtr& session, Protocol::EnterChatRoomReq& pkt)
{
    return false;
}
