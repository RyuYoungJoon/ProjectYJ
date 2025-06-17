#include "pch.h"
#include "ChatRoom.h"
#include "Player.h"
#include "GameSession.h"

ChatRoom GRoom;

std::string StringToUTF8(const std::string& wstr)
{
	int wideSize = MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0);
	std::wstring wideStr(wideSize, 0);
	MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), -1, &wideStr[0], wideSize);

	int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8Str(utf8Size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Size, nullptr, nullptr);

	return utf8Str;
}

void ChatRoom::Enter(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);
	m_ChatRoom.emplace(player->m_ChatRoomID, player);
}

void ChatRoom::Leave(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);
	m_ChatRoom.erase(player->m_ChatRoomID);
}

void ChatRoom::BroadCast(Packet packet)
{
	lock_guard<std::mutex> lock(m_ChatRoomMutex);

	for (auto& player : m_ChatRoom)
	{
		Packet packetCopy = packet;
		player.second->ownerSession->Send(std::move(packetCopy));
	}
}

void ChatRoom::SetRoomInfo(uint16 roomID, const std::string& roomName, uint16 maxUser)
{
	m_RoomID = roomID;
	m_RoomName = StringToUTF8(roomName);
	m_MaxUser = maxUser;
}

multimap<uint16, PlayerPtr> ChatRoom::GetChatRoom()
{
	return m_ChatRoom;
}

Protocol::ChatRoomInfo ChatRoom::GetRoomInfo()
{
	Protocol::ChatRoomInfo info;
	info.set_roomid(m_RoomID);
	info.set_currentuser(GetCurrentUser());
	info.set_maxuser(m_MaxUser);
	info.set_roomname(m_RoomName);

	return info;
}
