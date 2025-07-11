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
    return true;
}

bool HandleEnterChatRoomAck(AsioSessionPtr& session, Protocol::EnterChatRoomAck& pkt)
{
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return false;
    }

    Protocol::ChatRoomRes* data = new Protocol::ChatRoomRes();
    data->set_roomid(pkt.roomid());
    data->set_roomname(pkt.roomname());

    PostMessage(clientSession->s_hLobbyWin, WM_CLIENT_CHATROOM_ENTER, 0, (LPARAM)data);
}

bool HandleLoginAck(AsioSessionPtr& session, Protocol::LoginAck& pkt)
{
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return false;
    }

    LoginResponseData* loginData = new LoginResponseData();
    loginData->result = LOGIN_SUCCESS;
    loginData->userId = pkt.userid();
    loginData->message = "Login Success!";

    PostMessage(clientSession->s_hMainWin, WM_LOGIN_SUCCESS, 0, (LPARAM)loginData);
}

bool HandleChatRoomListAck(AsioSessionPtr& session, Protocol::ChatRoomListAck& pkt)
{
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return false;
    }

    Protocol::ChatRoomListRes* data = new Protocol::ChatRoomListRes();

    for (const auto& roomInfo : pkt.chatroominfo())
    {
        Protocol::ChatRoomInfo* newRoom = data->add_rooms();
        *newRoom = roomInfo;
    }

    ::PostMessage(clientSession->s_hLobbyWin, WM_CLIENT_CHATROOM_LIST, 0, (LPARAM)data);
}

bool HandleCreateChatRoomAck(AsioSessionPtr& session, Protocol::CreateChatRoomAck& pkt)
{
    return true;
}

bool HandleRefreshChatRoomAck(AsioSessionPtr& session, Protocol::RefreshChatRoomAck& pkt)
{
    return true;
}

bool HandleChatAck(AsioSessionPtr& session, Protocol::ChatAck& pkt)
{
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return false;
    }

    string name = pkt.sender();
    string message = pkt.message();

    Protocol::ChatMessageRes* data = new Protocol::ChatMessageRes();
    data->set_sender(pkt.sender());
    data->set_message(pkt.message());

    PostMessage(clientSession->s_hChatWin, WM_CLIENT_RECV, 0, (LPARAM)data);
}

bool HandleDummyPacketAck(AsioSessionPtr& session, Protocol::DummyPacketAck& pkt)
{
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return false;
    }

    string recvMessage = pkt.message();

    LOGI << "[ClientSessionUID : " << clientSession->GetSessionUID() << "] Recv Message " << recvMessage;

    return false;
}
