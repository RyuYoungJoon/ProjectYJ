#pragma once
class NetworkHandler
{
public:
	static NetworkHandler& GetInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	void RecvData(AsioSessionPtr session, BYTE* buffer, size_t length);

private:
	NetworkHandler();
	~NetworkHandler();
};

