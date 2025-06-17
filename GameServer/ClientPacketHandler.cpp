#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Logger.h"
#include "ServerAnalyzer.h"
#include "Player.h"
#include "ChatRoom.h"
#include "ChatRoomManager.h"
#include "ClientPacketHandler.h"


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
    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "GameSession Nullptr!";
        return false;
    }

    PlayerPtr player = gameSession->m_Player[0];
    player->m_ChatRoomID = pkt.roomid();

    ChatRoomPtr selectRoom = ChatRoomManager::GetInstance().GetRoom(pkt.roomid());

    selectRoom->Enter(player);

    LOGI << "Player " << player->m_Name << " entered the created chat room";

    Protocol::EnterChatRoomAck packet;
    packet.set_roomid(selectRoom->GetRoomID());
    packet.set_roomname(selectRoom->GetRoomName());
    Packet sendPacket = PacketHandler::MakePacket(packet);
    gameSession->Send(std::move(sendPacket));
}

bool HandleLoginReq(AsioSessionPtr& session, Protocol::LoginReq& pkt)
{
    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "GameSession Nullptr!";
        return false;
    }
    // DB 확인 로직 필요.

    PlayerPtr player = make_shared<Player>();
    player->m_PlayerUID = gameSession->GetSessionUID();
    player->ownerSession = gameSession;
    player->m_Name = pkt.id();

    gameSession->m_Player.push_back(player);

    Protocol::LoginAck loginPacket;

    loginPacket.set_success(true);
    loginPacket.set_userid(pkt.id());

    Packet packet = PacketHandler::MakePacket(loginPacket);

    session->Send(std::move(packet));
    return true;
}

bool HandleChatRoomListReq(AsioSessionPtr& session, Protocol::ChatRoomListReq& pkt)
{
    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (session == nullptr)
    {
        LOGE << "GameSession Nullptr";
        return false;
    }

    auto roomInfo = ChatRoomManager::GetInstance().GetAllRoom();

    Protocol::ChatRoomListAck sendPacket;

    for (const auto& tempRoom : roomInfo)
    {
        ChatRoomPtr room = tempRoom.second;
        Protocol::ChatRoomInfo info = room->GetRoomInfo();
        sendPacket.set_success(true);

        Protocol::ChatRoomInfo* newInfo = sendPacket.add_chatroominfo();
        *newInfo = info;
    }

    Packet packet = PacketHandler::MakePacket(sendPacket);

    gameSession->Send(std::move(packet));

    return true;
}

bool HandleCreateChatRoomReq(AsioSessionPtr& session, Protocol::CreateChatRoomReq& pkt)
{
    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "GameSession Nullptr";
        return false;
    }

    PlayerPtr player = gameSession->m_Player[0];

    ChatRoomPtr newRoom = ChatRoomManager::GetInstance().CreateRoom(pkt.roomname(), 10);

    uint32 roomID = newRoom->GetRoomID();

    LOGI << "Chat room created successfully: " << pkt.roomname() << " (ID: " << roomID << ")";

    newRoom->Enter(player);
    player->m_ChatRoomID = roomID;

    LOGI << "Player " << player->m_Name << " entered the created chat room";

    Protocol::EnterChatRoomAck packet;
    packet.set_roomid(roomID);

    packet.set_roomname(pkt.roomname());

    Packet sendPacket = PacketHandler::MakePacket(packet);
    gameSession->Send(std::move(sendPacket));
}
bool HandleRefreshChatRoomReq(AsioSessionPtr& session, Protocol::RefreshChatRoomReq& pkt)
{
    return true;
}

bool HandleChatReq(AsioSessionPtr& session, Protocol::ChatReq& pkt)
{
    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "GameSession Nullptr";
        return false;
    }

    PlayerPtr player = gameSession->m_Player[0];
    string message = pkt.message();

    ChatRoomPtr selectRoom = ChatRoomManager::GetInstance().GetRoom(player->m_ChatRoomID);

    Protocol::ChatAck packet;
    packet.set_sender(player->m_Name);
    packet.set_message(message);

    Packet sendPacket = PacketHandler::MakePacket(packet);
    // GRoom에 데이터 없슴. 왜냐? 여태껏 newRoom을 새로 생성해서 만들었거든.
    // 패킷에 roomID를 넣어서 selectRoom을 찾게 해야함.
    // 
    //if (gameSession->GetSessionUID() != pkt.sessionuid())
    selectRoom->BroadCast(sendPacket);
}
