#include "pch.h"
#include "SendPacketBuffer.h"
#include "AsioSession.h"
#include "PacketRouter.h"

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
        Packet* packet = PacketPool::GetInstance().AcquirePacket();

        // 패킷 데이터 복사
        buffer->OnRead(reinterpret_cast<BYTE*>(packet), header.size);

        // 패킷 디스패처로 전달
        PacketDispatcher::GetInstance().DispatchPacket(session, *packet);

        // 패킷 반환
        PacketPool::GetInstance().ReleasePacket(packet);
    }

    // 버퍼 정리
    buffer->Clear();
}
