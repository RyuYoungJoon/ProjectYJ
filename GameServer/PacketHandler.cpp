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

    // ó�� �޴� ��Ŷ�̸� ������ ��ȣ �ʱ�ȭ (0���� ����)
    if (m_NextSeq.find(sessionUID) == m_NextSeq.end())
    {
        m_NextSeq[sessionUID] = 0;
    }

    // ���� ���� ��Ŷ�� ������ ��ȣ
    int32 receivedSeqNum = packet->header.seqNum;
    int32 expectedSeqNum = m_NextSeq[sessionUID];

    if (receivedSeqNum == expectedSeqNum)
    {
        // ����� ������ ��ȣ�� ��ġ�ϸ� �ٷ� ó��
        auto it = m_Handlers.find(packet->header.type);
        if (it != m_Handlers.end())
        {
            it->second(session, packetRef);
        }
        else
        {
            HandleInvalid(session, packetRef);
        }

        // ���� ������ ��ȣ ������Ʈ
        m_NextSeq[sessionUID]++;
    }
    else if (receivedSeqNum > expectedSeqNum)
    {
        // ����� �ͺ��� ���� ������ ��ȣ�� �޾�����, ��� ť�� ����
        //LOGE << "������ ó�� ����! Expected: " << expectedSeqNum
           // << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    }
    else
    {
        // �̹� ó���� ��Ŷ�� ��� (receivedSeqNum < expectedSeqNum)
       // LOGD << "�̹� ó���� ��Ŷ! Expected: " << expectedSeqNum
           // << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    }
}

void PacketHandler::ProcessPendingPacket(AsioSessionPtr& session, int32 sessionUID)
{
    // ��������� ��� ���� ��Ŷ ó��
    auto& queue = m_PendingPacket[sessionUID];
    int32& expectedSeqNum = m_NextSeq[sessionUID];

    while (!queue.empty())
    {
        const Packet* frontPacket = queue.front();

        if (frontPacket->header.seqNum == expectedSeqNum)
        {
            queue.pop();

            // ��Ŷ ó��
            auto it = m_Handlers.find(frontPacket->header.type);
            if (it != m_Handlers.end())
            {
                it->second(session, frontPacket);
            }
            else
            {
                HandleInvalid(session, frontPacket);
            }

            // ���� ��� ������ ��ȣ ������Ʈ
            expectedSeqNum++;
        }
        else
        {
            // ���� ������ ���� �ʴ� ��Ŷ�̸� ����
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
	// �߰� ó�� ����

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
