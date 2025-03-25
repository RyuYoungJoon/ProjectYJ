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

	// 데이터 수신 카운트 증가.
	ServerAnalyzer::GetInstance().IncrementRecvCnt();

	// send버퍼에 데이터 add
	SessionPacketBuffer::GetInstance().AddData(session, buffer, length);

	// 버퍼 처리.
	SessionPacketBuffer::GetInstance().ProcessSessionBuffer(session, length);
}

void NetworkHandler::SendPacket(AsioSessionPtr session, const Packet& packet)
{
	if (!session)
		return;

	ServerAnalyzer::GetInstance().IncrementSendCnt();

	// 세션의 send 호출.
	session->Send(packet);
}
