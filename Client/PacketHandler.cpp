#include "pch.h"
#include "PacketHandler.h"
#include "ClientSession.h"

atomic<int> a;
PacketHandler::PacketHandler()
    : PacketProcessor()
{
    Init();
}
PacketHandler::~PacketHandler()
{
}
void PacketHandler::Init()
{
    RegisterHandler(PacketType::LoginAck, std::bind(&PacketHandler::HandleLoginAck, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::ChatAck, std::bind(&PacketHandler::HandleChatAck, this, std::placeholders::_1, std::placeholders::_2));
}

void PacketHandler::RegisterHandler(PacketType packetType, HandlerFunc handler)
{
    auto iter = m_Handlers.find(packetType);
    if (iter == m_Handlers.end())
        m_Handlers.emplace(packetType, handler);

    auto iter2 = m_RecvCount.find(packetType);
    if (iter2 == m_RecvCount.end())
        m_RecvCount.emplace(packetType, 0);
}

void PacketHandler::HandlePacket(AsioSessionPtr session, BYTE* buffer)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    if (!header || !session)
        return;

    int32 sessionUID = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    // 처음 받는 패킷이면 시퀀스 번호 초기화 (0부터 시작)
    if (m_NextSeq.find(sessionUID) == m_NextSeq.end())
    {
        m_NextSeq[sessionUID] = 0;
    }

    auto it = m_Handlers.find(header->type);
    if (it != m_Handlers.end())
    {
        it->second(session, buffer);
    }
    else
    {
        HandleInvalid(session, buffer);
    }
}

void PacketHandler::HandleLoginAck(AsioSessionPtr& session, BYTE* buffer)
{
    PacketLoginAck* packet = reinterpret_cast<PacketLoginAck*>(buffer);

    if (packet->header.type != PacketType::LoginAck)
        return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    //PostMessage(clientSession->s_hMainWin, WM_CLIENT_LOGIN, 0, (LPARAM)chatMessage);
}

void PacketHandler::HandleChatAck(AsioSessionPtr& session, BYTE* buffer)
{
    PacketChatAck* packet = reinterpret_cast<PacketChatAck*>(buffer);
    if (packet->header.type != PacketType::ChatAck)
        return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    std::string sender = "Server";

    ChatMessageData* chatMessage = new ChatMessageData();
    chatMessage->sender = packet->payload.sender;
    chatMessage->message = packet->payload.message;

    PostMessage(clientSession->s_hMainWin, WM_CLIENT_RECV, 0, (LPARAM)chatMessage);
}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_NextSeq.erase(sessionUID);
}


void PacketHandler::HandleInvalid(AsioSessionPtr& session, BYTE* buffer)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    LOGE << "Unknown Packet Type : " << static_cast<int16>(header->type);
}
