#pragma once
class ChatRoomManager
{
public:
	static ChatRoomManager& GetInstance()
	{
		static ChatRoomManager instance;
		return instance;
	}

	ChatRoomPtr CreateRoom(const std::string& name, uint16 maxUser = 10);
	void RemoveRoom(uint16 roomID);
	ChatRoomPtr GetRoom(uint16 roomID);
	std::map<uint16, ChatRoomPtr> GetAllRoom();

private:
	ChatRoomManager();
	~ChatRoomManager();

	std::map<uint16, ChatRoomPtr> m_ChatRoom;
	std::mutex m_ChatRoomMutex;
	atomic<uint16> roomNumber;
};

