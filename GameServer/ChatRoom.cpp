#include "pch.h"
#include "ChatRoom.h"
#include "Player.h"
#include "GameSession.h"

ChatRoom GRoom;

void ChatRoom::Enter(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_playerMutex);
	auto iter = m_Player.find(player->playerUID);
	if (iter == m_Player.end())
		m_Player.emplace(player->playerUID, player);
}

void ChatRoom::Leave(PlayerPtr player)
{
	lock_guard<std::mutex> lock(m_playerMutex);
	
	auto iter = m_Player.find(player->playerUID);
	if (iter == m_Player.end())
		m_Player.erase(player->playerUID);
}

void ChatRoom::BroadCast(const std::string& message)
{
	lock_guard<std::mutex> lock(m_playerMutex);

	for (auto& player : m_Player)
	{
		player.second->ownerSession->Send(message, PacketType::ChatAck);
	}

}
