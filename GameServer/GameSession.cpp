#include "pch.h"
#include "GameSession.h"
#include "PacketHandler.h"

GameSession::GameSession(boost::asio::io_context& iocontext, tcp::socket socket)
	: AsioSession(iocontext, std::move(socket))
{
	m_PacketHandler.Init();
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend ȣ��" << endl;
}

void GameSession::OnDisconnected()
{
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	Packet* packet = reinterpret_cast<Packet*>(buffer);
	m_PacketHandler.HandlePacket(packet);

	// ���� �ʱ�ȭ
	if(packet->tail.value = 255)
		m_PacketBuffer.DiscardReadData();

	return int32();
}

void GameSession::OnConnected()
{
}
