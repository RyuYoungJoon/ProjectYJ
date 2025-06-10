#pragma once
class ChatRoom
{
public:
	void Enter(PlayerPtr player);
	void Leave(PlayerPtr player);

	void BroadCast(const std::string& message);

	void SetRoomInfo(uint16 roomID, const std::string& roomName, uint16 maxUser);
	uint16 GetRoomID() const { return m_RoomID; }
	const std::string& GetRoomName() const { return m_RoomName; }
	uint16 GetCurrentUser() const { return m_ChatRoom.size(); }
	uint16 GetMaxUser() const { return m_MaxUser; }

	map<uint16, PlayerPtr> GetChatRoom();

	ChatRoomInfo GetRoomInfo();


private:
	std::map<uint16, PlayerPtr>m_ChatRoom;
	std::mutex m_ChatRoomMutex;
	uint16 m_RoomID = 0;
	std::string m_RoomName;
	uint16 m_MaxUser;
};

extern ChatRoom GRoom;