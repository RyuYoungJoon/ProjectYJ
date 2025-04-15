#pragma once
class Player
{
public:
	Player();
	~Player();

	uint64 playerUID = 0;
	string name;

	GameSessionPtr ownerSession;
};

