#include "pch.h"
#include "NetworkHandler.h"
#include "ServerAnalyzer.h"
#include "SessionPacketBuffer.h"
#include "AsioSession.h"

NetworkHandler::NetworkHandler()
{
}

NetworkHandler::~NetworkHandler()
{
}

void NetworkHandler::RecvData(AsioSessionPtr session, BYTE* buffer, size_t length)
{
	if (!session || !buffer || length == 0)
		return;

	// ������ ���� ī��Ʈ ����.
	ServerAnalyzer::GetInstance().IncrementRecvCnt();

	// send���ۿ� ������ add
	SessionPacketBuffer::GetInstance().AddData(session, buffer, length);

	// ���� ó��.
	SessionPacketBuffer::GetInstance().ProcessSessionBuffer(session, length);
}

void NetworkHandler::SendPacket(AsioSessionPtr session, const Packet& packet)
{
	if (!session)
		return;

	ServerAnalyzer::GetInstance().IncrementSendCnt();

	// ������ send ȣ��.
	session->Send(packet);
}
