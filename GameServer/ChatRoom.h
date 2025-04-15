#pragma once
class ChatRoom
{
public:
	void Enter(PlayerPtr player);
	void Leave(PlayerPtr player);
	void BroadCast(const std::string& message);

private:
	std::map<uint64, PlayerPtr>m_Player;
	std::mutex m_playerMutex;
};

extern ChatRoom GRoom;