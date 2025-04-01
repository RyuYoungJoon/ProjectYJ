#include "pch.h"
#include "SessionPacketBuffer.h"
#include "AsioSession.h"
#include "PacketRouter.h"
#include "ObjectPool.h"

SessionPacketBuffer::SessionPacketBuffer()
{
}

SessionPacketBuffer::~SessionPacketBuffer()
{
}

void SessionPacketBuffer::AddData(AsioSessionPtr session, const BYTE* data, size_t length)
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
        session->ProcessDisconnect(__FUNCTION__);
    }
}

void SessionPacketBuffer::ProcessSessionBuffer(AsioSessionPtr session, size_t length)
{
    if (!session)
        return;

    int32 sessionId = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
        return;

    // 버퍼 정리
    PacketBuffer* buffer = it->second.get();

    int32 processLen = session->ProcessPacket(buffer->ReadPos(), buffer->DataSize());

    if (processLen <= 0 || buffer->OnRead(processLen) == false)
    {
        LOGE << "Buffer OverFlow!!";
        return;
    }

    buffer->Clear();
}
