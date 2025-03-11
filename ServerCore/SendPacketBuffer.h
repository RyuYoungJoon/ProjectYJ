#pragma once
#include "PacketBuffer.h"
// SendPacketBuffer: 세션별 패킷 버퍼 관리
class SendPacketBuffer
{
public:
    static SendPacketBuffer& GetInstance()
    {
        static SendPacketBuffer instance;
        return instance;
    }

    void AddData(AsioSessionPtr session, const BYTE* data, size_t length);
    void ProcessSessionBuffer(AsioSessionPtr session);

private:
    SendPacketBuffer();
    ~SendPacketBuffer();

    std::mutex m_Mutex;
    std::unordered_map<int32, std::unique_ptr<PacketBuffer>> m_SessionBuffers;
};
