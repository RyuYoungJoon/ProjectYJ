#include "pch.h"
#include "SendPacketBuffer.h"
#include "AsioSession.h"
#include "PacketRouter.h"
#include "ObjectPool.h"

SendPacketBuffer::SendPacketBuffer()
{
    m_PacketPool.InitPool(10000);
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

void SendPacketBuffer::ProcessSessionBuffer(AsioSessionPtr session, size_t length)
{
    if (!session)
        return;

    int32 sessionId = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
        return;

    // 패킷 처리
    //while (buffer->DataSize() >= sizeof(PacketHeader))
    //{
    //    // 헤더만 먼저 확인
    //    PacketHeader header;
    //    memcpy(&header, buffer->ReadPos(), sizeof(PacketHeader));

    //    // 패킷 완성 체크
    //    if (buffer->DataSize() < header.size)
    //        break;

    //    // 패킷 풀에서 패킷 가져오기
    //    shared_ptr<Packet> packet = m_PacketPool.Pop();
    //    memcpy(&packet->header, buffer->ReadPos(), sizeof(PacketHeader));

    //    if (header.size > sizeof(PacketHeader))
    //        memcpy(packet->payload, buffer->ReadPos() + sizeof(PacketHeader), header.size - sizeof(PacketHeader));

    //    // 풀에서 가져온 Packet에는 아직 아무 값이 없는데
    //    // 이 packet 값을 buffer에 넣는 무의미한 작업.
    //    // packet에는 아무 값도 없는데 buffer에 덮고 패킷 라우터로 Dispatch하는중.
    //    // 수정 꼭 필요.
    //    // 
    //    // 패킷 데이터 복사
    //    
    //    // 패킷 디스패처로 전달
    //    PacketRouter::GetInstance().Dispatch(session, *packet);
    //    
    //    buffer->OnRead(header.size);
    //    buffer->Clear();

    //    // 패킷 풀 반환은 스마트 포인터이기 때문에 자동 Push된다.
    //}

    //int32 processLen = 0;
    //while (true)
    //{
    //    int32 dataSize = length - processLen;
    //    if (dataSize < sizeof(PacketHeader))
    //        break;

    //    PacketHeader header = 
    //}

    // 버퍼 정리
    PacketBuffer* buffer = it->second.get();

    int32 processLen = session->ProcessPacket(buffer->ReadPos(), buffer->DataSize());

    if (processLen > 0)
    {
        buffer->OnRead(processLen);
    }

    buffer->Clear();

}
