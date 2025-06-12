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
    LOGI << "����!";
    return true;
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

    ChatRoomManager::GetInstance().CreateRoom("HIHIHI", 10);
    ChatRoomManager::GetInstance().CreateRoom("hihihi333", 10);
    ChatRoomManager::GetInstance().CreateRoom("한글은?", 10);

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
    return true;
}
bool HandleRefreshChatRoomReq(AsioSessionPtr& session, Protocol::RefreshChatRoomReq& pkt)
{
    return true;
}