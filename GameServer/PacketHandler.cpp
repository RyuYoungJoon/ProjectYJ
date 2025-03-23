#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "Logger.h"

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
}

void PacketHandler::RegisterHandler(PacketType packetType, HandlerFunc handler)
{
	auto iter = m_Handlers.find(packetType);
	if (iter == m_Handlers.end())
		m_Handlers.emplace(packetType, handler);
}

void PacketHandler::HandlePacket(AsioSessionPtr session, const Packet* packet)
{
    if (!packet || !session)
        return;

    int32 sessionUID = session->GetSessionUID();
    const Packet* packetRef = packet;

    std::lock_guard<std::mutex> lock(m_Mutex);

    // 처음 받는 패킷이면 시퀀스 번호 초기화 (0부터 시작)
    if (m_NextSeq.find(sessionUID) == m_NextSeq.end())
    {
        m_NextSeq[sessionUID] = 0;
    }

    // 현재 받은 패킷의 시퀀스 번호
    int32 receivedSeqNum = packet->header.seqNum;
    int32 expectedSeqNum = m_NextSeq[sessionUID];

    if (receivedSeqNum == expectedSeqNum)
    {
        // 기대한 시퀀스 번호와 일치하면 바로 처리
        auto it = m_Handlers.find(packet->header.type);
        if (it != m_Handlers.end())
        {
            it->second(session, packetRef);
        }
        else
        {
            HandleInvalid(session, packetRef);
        }

        // 다음 시퀀스 번호 업데이트
        m_NextSeq[sessionUID]++;
    }
    else if (receivedSeqNum > expectedSeqNum)
    {
        // 기대한 것보다 높은 시퀀스 번호를 받았으면, 대기 큐에 저장
        //LOGE << "시퀀스 처리 에러! Expected: " << expectedSeqNum
           // << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    }
    else
    {
        // 이미 처리한 패킷인 경우 (receivedSeqNum < expectedSeqNum)
       // LOGD << "이미 처리한 패킷! Expected: " << expectedSeqNum
           // << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    }
}

void PacketHandler::ProcessPendingPacket(AsioSessionPtr& session, int32 sessionUID)
{
    // 재귀적으로 대기 중인 패킷 처리
    auto& queue = m_PendingPacket[sessionUID];
    int32& expectedSeqNum = m_NextSeq[sessionUID];

    while (!queue.empty())
    {
        const Packet* frontPacket = queue.front();

        if (frontPacket->header.seqNum == expectedSeqNum)
        {
            queue.pop();

            // 패킷 처리
            auto it = m_Handlers.find(frontPacket->header.type);
            if (it != m_Handlers.end())
            {
                it->second(session, frontPacket);
            }
            else
            {
                HandleInvalid(session, frontPacket);
            }

            // 다음 기대 시퀀스 번호 업데이트
            expectedSeqNum++;
        }
        else
        {
            // 아직 순서가 맞지 않는 패킷이면 종료
            break;
        }
    }
}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_PendingPacket.erase(sessionUID);
    m_NextSeq.erase(sessionUID);
}

void PacketHandler::Test()
{
    //LOGD << "PacketHandler";
}

void PacketHandler::HandledefEchoString(AsioSessionPtr& session, const Packet* packet)
{
	if (packet->header.type != PacketType::defEchoString)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	//LOGD << "SessionUID : "<<gameSession->GetSessionUID()<<", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
}

void PacketHandler::HandleJH(AsioSessionPtr& session, const Packet* packet)
{
	if (packet->header.type != PacketType::JH)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	//LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
	// 추가 처리 로직

}

void PacketHandler::HandleYJ(AsioSessionPtr& session, const Packet* packet)
{
	if (packet->header.type != PacketType::YJ)
		return;
    a.fetch_add(1);
	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

    //LOGD << "[" << a << "]SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet->header.seqNum << "] -> Payload : " << packet->payload;
}

void PacketHandler::HandleES(AsioSessionPtr& session, const Packet* packet)
{
	if (packet->header.type != PacketType::ES)
		return;

	GameSessionPtr gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
	{
		LOGE << "Session Nullptr!";
		return;
	}

	//LOGD << "SessionUID : " << gameSession->GetSessionUID() << ", [Seq : " << packet.header.seqNum << "] -> Payload : " << packet.payload;
}

void PacketHandler::HandleInvalid(AsioSessionPtr& session, const Packet* packet)
{
	LOGE << "Unknown Packet Type : " << static_cast<int16>(packet->header.type);
}
