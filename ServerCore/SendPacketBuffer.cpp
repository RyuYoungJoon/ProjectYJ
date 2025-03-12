#include "pch.h"
#include "SendPacketBuffer.h"
#include "AsioSession.h"
#include "PacketRouter.h"
#include "ObjectPool.h"

SendPacketBuffer::SendPacketBuffer()
{
}

SendPacketBuffer::~SendPacketBuffer()
{
}


void SendPacketBuffer::AddData(AsioSessionPtr session, const BYTE* data, size_t length)
{
    if (!session || !data || length == 0)
        return;

    int32 sessionId = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    // 버퍼가 없으면 새로 생성
    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
    {
        m_SessionBuffers[sessionId] = std::make_unique<PacketBuffer>();
    }

    // 데이터 추가
    PacketBuffer* buffer = m_SessionBuffers[sessionId].get();
    if (!buffer->OnWrite(data, static_cast<int32>(length)))
    {
        // 버퍼 오버플로우 처리
        LOGE << "Buffer overflow for session " << sessionId;
        session->CloseSession();
    }
}

void SendPacketBuffer::ProcessSessionBuffer(AsioSessionPtr session)
{
    if (!session)
        return;

    int32 sessionId = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
        return;

    PacketBuffer* buffer = it->second.get();

    // 패킷 처리
    while (buffer->DataSize() >= sizeof(PacketHeader))
    {
        // 헤더만 먼저 확인
        PacketHeader header;
        memcpy(&header, buffer->ReadPos(), sizeof(PacketHeader));

        // 패킷 완성 체크
        if (buffer->DataSize() < header.size)
            break;

        // 패킷 풀에서 패킷 가져오기
        Packet* packet = PacketPool::GetInstance().Pop();
        memcpy(&packet->header, buffer->ReadPos(), sizeof(PacketHeader));

        if (header.size > sizeof(PacketHeader))
            memcpy(packet->payload, buffer->ReadPos() + sizeof(PacketHeader), header.size - sizeof(PacketHeader));

        // 풀에서 가져온 Packet에는 아직 아무 값이 없는데
        // 이 packet 값을 buffer에 넣는 무의미한 작업.
        // packet에는 아무 값도 없는데 buffer에 덮고 패킷 라우터로 Dispatch하는중.
        // 수정 꼭 필요.
        // 
        // 패킷 데이터 복사
        buffer->OnRead(header.size);

        // 패킷 디스패처로 전달
        PacketRouter::GetInstance().Dispatch(session, *packet);
        
        // 패킷 반환
        PacketPool::GetInstance().Push(packet);
    }

    // 버퍼 정리
    buffer->Clear();
}
