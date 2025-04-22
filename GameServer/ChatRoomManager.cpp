#include "pch.h"
#include "ChatRoomManager.h"
#include "ChatRoom.h"

ChatRoomPtr ChatRoomManager::CreateRoom(const std::string& name, uint16 maxUser)
{
	std::lock_guard<std::mutex> lock(m_ChatRoomMutex);

	roomNumber.fetch_add(1);
	ChatRoomPtr chatRoom = make_shared<ChatRoom>();
	chatRoom->SetRoomInfo(roomNumber, name, maxUser);

	m_ChatRoom[roomNumber] = chatRoom;
	return chatRoom;
}

void ChatRoomManager::RemoveRoom(uint16 roomID)
{
	std::lock_guard<std::mutex> lock(m_ChatRoomMutex);
	m_ChatRoom.erase(roomID);
}

ChatRoomPtr ChatRoomManager::GetRoom(uint16 roomID)
{
	std::lock_guard<std::mutex> lock(m_ChatRoomMutex);
	auto iter = m_ChatRoom.find(roomID);
	if (iter != m_ChatRoom.end())
		return iter->second;

	return nullptr;
}

std::map<uint16, ChatRoomPtr> ChatRoomManager::GetAllRoom()
{
	std::lock_guard<std::mutex> lock(m_ChatRoomMutex);
	return m_ChatRoom;
}

ChatRoomManager::ChatRoomManager()
{
	roomNumber.store(0);
}

ChatRoomManager::~ChatRoomManager()
{
}
