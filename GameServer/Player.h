#pragma once
class Player
{
public:
	Player();
	~Player();

	uint64 m_PlayerUID = 0;
	string m_Name;
	uint16 m_ChatRoomID = 0;

	GameSessionPtr ownerSession;
};

