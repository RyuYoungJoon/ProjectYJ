#include "pch.h"
#include "ChatRoom.h"
#include "Player.h"
#include "GameSession.h"

ChatRoom GRoom;

void ChatRoom::Enter(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);
	auto iter = m_ChatRoom.find(player->m_ChatRoomID);
	if (iter == m_ChatRoom.end())
		m_ChatRoom.emplace(player->m_ChatRoomID, player);
}

void ChatRoom::Leave(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);
	
	auto iter = m_ChatRoom.find(player->m_ChatRoomID);
	if (iter == m_ChatRoom.end())
		m_ChatRoom.erase(player->m_ChatRoomID);
}

void ChatRoom::BroadCast(const std::string& message)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);

	//PacketChatAck chatPacket;
	//chatPacket.payload.sender = "Server";
	//chatPacket.payload.message = message;

	for (auto& player : m_ChatRoom)
	{
		//player.second->ownerSession->Send(chatPacket);
	}
}

void ChatRoom::SetRoomInfo(uint16 roomID, const std::string& roomName, uint16 maxUser)
{
	m_RoomID = roomID;
	m_RoomName = roomName;
	m_MaxUser = maxUser;
}

map<uint16, PlayerPtr> ChatRoom::GetChatRoom()
{
	return m_ChatRoom;
}

ChatRoomInfo ChatRoom::GetRoomInfo()
{
	ChatRoomInfo info;
	info.roomID = m_RoomID;
	//info.roomName = m_RoomName;
	info.currentUser = GetCurrentUser();
	info.maxUser = m_MaxUser;

	return info;
}
