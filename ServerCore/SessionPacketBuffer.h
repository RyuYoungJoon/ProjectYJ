#pragma once
#include "PacketBuffer.h"
#include "ObjectPool.h"
// SessionPacketBuffer: 세션별 패킷 버퍼 관리
class SessionPacketBuffer
{
public:
    static SessionPacketBuffer& GetInstance()
    {
        static SessionPacketBuffer instance;
        return instance;
    }

    void AddData(AsioSessionPtr session, const BYTE* data, size_t length);
    void ProcessSessionBuffer(AsioSessionPtr session, size_t length);

private:
    SessionPacketBuffer();
    ~SessionPacketBuffer();

    std::mutex m_Mutex;
    std::unordered_map<int32, std::unique_ptr<PacketBuffer>> m_SessionBuffers;
};
