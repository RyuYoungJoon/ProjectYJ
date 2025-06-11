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
    RegisterHandler(PacketType::RoomEnterAck, std::bind(&PacketHandler::HandleRoomEnterAck, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::RoomListAck, std::bind(&PacketHandler::HandleRoomListAck, this, std::placeholders::_1, std::placeholders::_2));
    RegisterHandler(PacketType::RoomCreateAck, std::bind(&PacketHandler::HandleRoomCreateAck, this, std::placeholders::_1, std::placeholders::_2));
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
    //PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    //if (!header || !session)
     //   return;

    int32 sessionUID = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

   // auto it = m_Handlers.find(header->type);
    //if (it != m_Handlers.end())
    {
       // it->second(session, buffer);
    }
   // else
    {
    //    HandleInvalid(session, buffer);
    }
}

void PacketHandler::HandleLoginAck(AsioSessionPtr& session, BYTE* buffer)
{
    //PacketLoginAck* packet = reinterpret_cast<PacketLoginAck*>(buffer);

   // if (packet->header.type != PacketType::LoginAck)
    //    return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    // 서버 응답을 기다림 (WM_CLIENT_LOGIN 메시지로 처리)
    // 테스트를 위해 임시로 로그인 성공으로 처리
    LoginResponseData* LoginData = new LoginResponseData();
    LoginData->result = LOGIN_SUCCESS;
   // LoginData->userId = packet->payload.id;
    LoginData->message = "로그인 성공";

    PostMessage(clientSession->s_hMainWin, WM_LOGIN_SUCCESS, 0, (LPARAM)LoginData);
}

void PacketHandler::HandleChatAck(AsioSessionPtr& session, BYTE* buffer)
{
  //  PacketChatAck* packet = reinterpret_cast<PacketChatAck*>(buffer);
  //  if (packet->header.type != PacketType::ChatAck)
   //     return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    std::string sender = "Server";

    ChatMessageData* chatMessage = new ChatMessageData();
  //  chatMessage->sender = packet->payload.sender;
 //   chatMessage->message = packet->payload.message;

    PostMessage(clientSession->s_hChatWin, WM_CLIENT_RECV, 0, (LPARAM)chatMessage);
}

void PacketHandler::HandleRoomEnterAck(AsioSessionPtr& session, BYTE* buffer)
{
   // PacketRoomEnterAck* packet = reinterpret_cast<PacketRoomEnterAck*>(buffer);
   // if (packet->header.type != PacketType::RoomEnterAck)
    //    return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    // 채팅창으로 전환 메시지 전송
    // 선택한 채팅방 ID와 이름 정보를 담은 구조체 생성
    ChatRoomResponseData* data = new ChatRoomResponseData();
  //  data->roomId = packet->payload.roomID;
   // data->roomName = packet->payload.roomName;
    
    ::PostMessage(clientSession->s_hChatWin, WM_ENTER_CHATROOM, 0, (LPARAM)data);
}

void PacketHandler::HandleRoomListAck(AsioSessionPtr& session, BYTE* buffer)
{
  //  PacketRoomListAck* packet = reinterpret_cast<PacketRoomListAck*>(buffer);
   // if (packet->header.type != PacketType::RoomListAck)
   //     return;

    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    ChatRoomListResponseData* data = new ChatRoomListResponseData();
    //data->rooms = packet->payload.chatRoomInfo;
  //  data->rooms = std::move(packet->payload.chatRoomInfo);

    ::PostMessage(clientSession->s_hLobbyWin, WM_CLIENT_CHATROOM_LIST, 0, (LPARAM)data);
}

void PacketHandler::HandleRoomCreateAck(AsioSessionPtr& session, BYTE* buffer)
{
   // PacketRoomCreateAck* packet = reinterpret_cast<PacketRoomCreateAck*>(buffer);
  //  if (packet->header.type != PacketType::RoomCreateAck)
   //     return;
    
    ClientSessionPtr clientSession = static_pointer_cast<ClientSession>(session);
    if (clientSession == nullptr)
    {
        LOGE << "Session Nullptr!";
        return;
    }

    ChatRoomResponseData* data = new ChatRoomResponseData();
   // data->roomId = packet->payload.roomID;
   // data->roomName = packet->payload.roomName;

    ::PostMessage(clientSession->s_hLobbyWin, WM_CLIENT_CHATROOM_CREATE, 0, (LPARAM)data);
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
