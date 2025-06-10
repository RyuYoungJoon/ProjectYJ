#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "Logger.h"
#include "ServerAnalyzer.h"
#include "Player.h"
#include "ChatRoom.h"
#include "ChatRoomManager.h"


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
    RegisterHandler(PacketType::StressTestPacket, std::bind(&PacketHandler::HandleDummyClient, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::ChatReq, std::bind(&PacketHandler::HandleChatReq, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::LoginReq, std::bind(&PacketHandler::HandleLoginReq, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::RoomEnterReq, std::bind(&PacketHandler::HandleRoomEnterReq , this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::RoomCreateReq, std::bind(&PacketHandler::HandleRoomCreateReq, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::RoomListReq, std::bind(&PacketHandler::HandleRoomListReq, this, std::placeholders::_1, std::placeholders::_2));
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
    Protocol::PacketHeader* header = reinterpret_cast<Protocol::PacketHeader*>(buffer);

    if (!header || !session)
        return;

    int32 sessionUID = session->GetSessionUID();
    
    std::lock_guard<std::mutex> lock(m_Mutex);

    // 처음 받는 패킷이면 시퀀스 번호 초기화 (0부터 시작)
    if (m_NextSeq.find(sessionUID) == m_NextSeq.end())
    {
        m_NextSeq[sessionUID] = 0;
    }

}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_NextSeq.erase(sessionUID);
}

void PacketHandler::HandleDummyClient(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketChatReq* packet = reinterpret_cast<PacketChatReq*>(buffer);

    //if (packet->header.type != PacketType::ChatReq)
       // return;


    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    ServerAnalyzer::GetInstance().IncrementRecvCnt();

    // 받은 횟수 카운트
    LOGD << "RecvCount : " << ServerAnalyzer::GetInstance().GetRecvCount() 
        << ", TotalRecvCount : " << ServerAnalyzer::GetInstance().GetTotalRecvCount();
}

void PacketHandler::HandleChatReq(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketChatReq* packet = reinterpret_cast<PacketChatReq*>(buffer);

    //if (packet->header.type != PacketType::ChatReq)
        //return;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    //LOGD << "Client [" <<gameSession->GetSessionUID()<<"] -> " << "Send Message : " << packet->payload.message;
    
    //PacketChatAck ackPacket;
    //ackPacket.payload.message = packet->payload.message;

    //GRoom.BroadCast(ackPacket.payload.message);
}

void PacketHandler::HandleLoginReq(AsioSessionPtr& session, BYTE* buffer)
{
   // PacketLoginReq* packet = reinterpret_cast<PacketLoginReq*>(buffer);

    //if (packet->header.type != PacketType::LoginReq)
        //return;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    LOGD << "Login Sucess!";

    PlayerPtr player = make_shared<Player>();
    player->m_PlayerUID = gameSession->GetSessionUID();
    player->ownerSession = gameSession;
    player->m_Name = "Player1";

    gameSession->m_Player.push_back(player);

    //GRoom.Enter(player);

   // PacketLoginAck ackPacket;
    //ackPacket.header.type = PacketType::LoginAck;
    //ackPacket.payload.id = player->m_Name;

    //gameSession->Send(ackPacket);
}

void PacketHandler::HandleRoomEnterReq(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketRoomEnterReq* packet = reinterpret_cast<PacketRoomEnterReq*>(buffer);

    //if (packet->header.type != PacketType::RoomEnterReq)
        //r/eturn;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    ChatRoomPtr chatRoom = make_shared<ChatRoom>();

    //PacketRoomEnterAck sendPacket;
    //sendPacket.header.type = PacketType::RoomEnterAck;
    //sendPacket.payload.roomID = 1;
    //sendPacket.payload.roomName = "하이";
    //sendPacket.payload.message = "안녕하세요.";
    //
    //gameSession->Send(sendPacket);
}

void PacketHandler::HandleRoomCreateReq(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketRoomCreateReq* packet = reinterpret_cast<PacketRoomCreateReq*>(buffer);
    //if (packet->header.type != PacketType::RoomCreateReq)
    //    return;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    //ChatRoomPtr newRoom = ChatRoomManager::GetInstance().CreateRoom(packet->payload.roomName, 10);

    //LOGI << "CreateRoom : " << packet->payload.roomName << ", ID : " << newRoom->GetRoomID();

    //PacketRoomCreateAck ackPacket;
    //ackPacket.header.type = PacketType::RoomCreateAck;
    //ackPacket.payload.roomID = newRoom->GetRoomID();
    //ackPacket.payload.roomName = newRoom->GetRoomName();
    //
    //gameSession->Send(ackPacket);
}

void PacketHandler::HandleRoomListReq(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketRoomListReq* packet = reinterpret_cast<PacketRoomListReq*>(buffer);

    //if (packet->header.type != PacketType::RoomListReq)
        //return;

    LOGI << "RoomListReq!";

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    auto roomInfo = ChatRoomManager::GetInstance().GetAllRoom();

    //PacketRoomListAck sendPacket;
    //sendPacket.header.type = PacketType::RoomListAck;

    for (const auto& tempRoom : roomInfo)
    {
        ChatRoomPtr room = tempRoom.second;
        auto info = room->GetRoomInfo();
        LOGI <<"RoomID : " << info.roomID 
            //<< ", RoomName : " << info.roomName 
            << ", CurrentUser : " << info.currentUser 
            << ", MaxUser" << info.maxUser;
        //sendPacket.payload.chatRoomInfo.push_back(room->GetRoomInfo());
    }

    //LOGD << sizeof(sendPacket.header) + sizeof(sendPacket.payload) + sizeof(sendPacket.tail);

    //gameSession->Send(sendPacket);
}
