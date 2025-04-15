#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "Logger.h"
#include "Player.h"
#include "ChatRoom.h"

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
	RegisterHandler(PacketType::defEchoString, std::bind(&PacketHandler::HandledefEchoString, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::JH, std::bind(&PacketHandler::HandleJH, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::YJ, std::bind(&PacketHandler::HandleYJ, this, std::placeholders::_1, std::placeholders::_2));
	RegisterHandler(PacketType::ES, std::bind(&PacketHandler::HandleES, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::ChatReq, std::bind(&PacketHandler::HandleChatReq, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::LoginReq, std::bind(&PacketHandler::HandleLoginReq, this, std::placeholders::_1, std::placeholders::_2));
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

void PacketHandler::HandlePacket(AsioSessionPtr session, Packet* packet)
{
    if (!packet || !session)
        return;

    int32 sessionUID = session->GetSessionUID();
    Packet* packetRef = packet;

    std::lock_guard<std::mutex> lock(m_Mutex);

    // 처음 받는 패킷이면 시퀀스 번호 초기화 (0부터 시작)
    if (m_NextSeq.find(sessionUID) == m_NextSeq.end())
    {
        m_NextSeq[sessionUID] = 0;
    }

    auto it = m_Handlers.find(packet->header.type);
    if (it != m_Handlers.end())
    {
        it->second(session, packetRef);
    }
    else
    {
        HandleInvalid(session, packetRef);
    }

    // 현재 받은 패킷의 시퀀스 번호
    //int32 receivedSeqNum = packet->header.seqNum;
    //int32 expectedSeqNum = m_NextSeq[sessionUID];

    //if (receivedSeqNum == expectedSeqNum)
    //{
    //    // 기대한 시퀀스 번호와 일치하면 바로 처리
    //    auto it = m_Handlers.find(packet->header.type);
    //    if (it != m_Handlers.end())
    //    {
    //        it->second(session, packetRef);
    //    }
    //    else
    //    {
    //        HandleInvalid(session, packetRef);
    //    }

    //    // 다음 시퀀스 번호 업데이트
    //    m_NextSeq[sessionUID]++;
    //}
    //else if (receivedSeqNum > expectedSeqNum)
    //{
    //    // 기대한 것보다 높은 시퀀스 번호를 받았으면
    //    LOGE << "시퀀스 처리 에러! Expected: " << expectedSeqNum
    //        << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    //}
    //else
    //{
    //    // 이미 처리한 패킷인 경우 (receivedSeqNum < expectedSeqNum)
    //    LOGD << "이미 처리한 패킷! Expected: " << expectedSeqNum
    //        << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    //}
}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_NextSeq.erase(sessionUID);
}

void PacketHandler::HandledefEchoString(AsioSessionPtr& session, Packet* packet)
{
	if (packet->header.type != PacketType::defEchoString)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

    auto iter = m_RecvCount.find(packet->header.type);
    if (iter != m_RecvCount.end())
        iter->second.fetch_add(1);

	LOGD << "SessionUID : "<<gameSession->GetSessionUID()<<", [Seq : " << packet->header.seqNum << "] -> Payload : " << packet->payload
        << ", RecvCount : " << iter->second;
}

void PacketHandler::HandleJH(AsioSessionPtr& session, Packet* packet)
{
	if (packet->header.type != PacketType::JH)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

    auto iter = m_RecvCount.find(packet->header.type);
    if (iter != m_RecvCount.end())
        iter->second.fetch_add(1);

	LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet->header.seqNum << "] -> Payload : " << packet->payload
        << ", RecvCount : " << iter->second;
	// 추가 처리 로직

}

void PacketHandler::HandleYJ(AsioSessionPtr& session, Packet* packet)
{
	if (packet->header.type != PacketType::YJ)
		return;
    
	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

    auto iter = m_RecvCount.find(packet->header.type);
    if (iter != m_RecvCount.end())
        iter->second.fetch_add(1);

    LOGD << "[" << a << "]SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet->header.seqNum << "] -> Payload : " << packet->payload
        << ", RecvCount : " << iter->second;
}

void PacketHandler::HandleES(AsioSessionPtr& session, Packet* packet)
{
	if (packet->header.type != PacketType::ES)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

    auto iter = m_RecvCount.find(packet->header.type);
    if (iter != m_RecvCount.end())
        iter->second.fetch_add(1);

	LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet->header.seqNum << "] -> Payload : " << packet->payload
        << ", RecvCount : " << iter->second;
}

void PacketHandler::HandleChatReq(AsioSessionPtr& session, Packet* packet)
{
    if (packet->header.type != PacketType::ChatReq)
        return;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    LOGD << "Client [" <<gameSession->GetSessionUID()<<"] -> " << "Send Message : " << packet->payload;
    std::string message(reinterpret_cast<char*>(packet->payload), sizeof(packet->payload));

    GRoom.BroadCast(message);
}

void PacketHandler::HandleLoginReq(AsioSessionPtr& session, Packet* packet)
{
    if (packet->header.type != PacketType::LoginReq)
        return;

    GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    LOGD << "Login Sucess!";

    PlayerPtr player = make_shared<Player>();
    player->playerUID = gameSession->GetSessionUID();
    player->ownerSession = gameSession;
    player->name = "Player1";

    gameSession->m_Player.push_back(player);

    GRoom.Enter(player);

    gameSession->Send("LoginAck", PacketType::LoginAck);
}

void PacketHandler::HandleInvalid(AsioSessionPtr& session, Packet* packet)
{
	LOGE << "Unknown Packet Type : " << static_cast<int16>(packet->header.type);
}
