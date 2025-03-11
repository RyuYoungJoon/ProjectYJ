#pragma once
#include "PacketBuffer.h"
// SendPacketBuffer: ���Ǻ� ��Ŷ ���� ����
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
