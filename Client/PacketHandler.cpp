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

void PacketHandler::HandlePacket(AsioSessionPtr session, Packet* packet)
{
    if (!packet || !session)
        return;

    int32 sessionUID = session->GetSessionUID();
    Packet* packetRef = packet;

    std::lock_guard<std::mutex> lock(m_Mutex);

    // ó�� �޴� ��Ŷ�̸� ������ ��ȣ �ʱ�ȭ (0���� ����)
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

    // ���� ���� ��Ŷ�� ������ ��ȣ
    //int32 receivedSeqNum = packet->header.seqNum;
    //int32 expectedSeqNum = m_NextSeq[sessionUID];

    //if (receivedSeqNum == expectedSeqNum)
    //{
    //    // ����� ������ ��ȣ�� ��ġ�ϸ� �ٷ� ó��
    //    auto it = m_Handlers.find(packet->header.type);
    //    if (it != m_Handlers.end())
    //    {
    //        it->second(session, packetRef);
    //    }
    //    else
    //    {
    //        HandleInvalid(session, packetRef);
    //    }

    //    // ���� ������ ��ȣ ������Ʈ
    //    m_NextSeq[sessionUID]++;
    //}
    //else if (receivedSeqNum > expectedSeqNum)
    //{
    //    // ����� �ͺ��� ���� ������ ��ȣ�� �޾�����
    //    LOGE << "������ ó�� ����! Expected: " << expectedSeqNum
    //        << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    //}
    //else
    //{
    //    // �̹� ó���� ��Ŷ�� ��� (receivedSeqNum < expectedSeqNum)
    //    LOGD << "�̹� ó���� ��Ŷ! Expected: " << expectedSeqNum
    //        << ", Received: " << receivedSeqNum << ", SessionUID: " << sessionUID;
    //}
}

void PacketHandler::HandleLoginAck(AsioSessionPtr& session, Packet* packet)
{
    if (packet->header.type != PacketType::LoginAck)
        return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    
}

void PacketHandler::HandleChatAck(AsioSessionPtr& session, Packet* packet)
{
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
    chatMessage->sender = sender;
    chatMessage->message = reinterpret_cast<char*>(packet->payload);

    PostMessage(clientSession->s_hMainWin, WM_CLIENT_RECV, 0, (LPARAM)chatMessage);
}

void PacketHandler::Reset(int32 sessionUID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_NextSeq.erase(sessionUID);
}


void PacketHandler::HandleInvalid(AsioSessionPtr& session, Packet* packet)
{
    LOGE << "Unknown Packet Type : " << static_cast<int16>(packet->header.type);
}
