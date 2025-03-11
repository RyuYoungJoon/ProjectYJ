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

    // ���۰� ������ ���� ����
    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
    {
        m_SessionBuffers[sessionId] = std::make_unique<PacketBuffer>();
    }

    // ������ �߰�
    PacketBuffer* buffer = m_SessionBuffers[sessionId].get();
    if (!buffer->OnWrite(data, static_cast<int32>(length)))
    {
        // ���� �����÷ο� ó��
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

    // ��Ŷ ó��
    while (buffer->DataSize() >= sizeof(PacketHeader))
    {
        // ����� ���� Ȯ��
        PacketHeader header;
        memcpy(&header, buffer->ReadPos(), sizeof(PacketHeader));

        // ��Ŷ �ϼ� üũ
        if (buffer->DataSize() < header.size)
            break;

        // ��Ŷ Ǯ���� ��Ŷ ��������
        Packet* packet = PacketPool::GetInstance().AcquirePacket();

        // ��Ŷ ������ ����
        buffer->OnRead(reinterpret_cast<BYTE*>(packet), header.size);

        // ��Ŷ ����ó�� ����
        PacketDispatcher::GetInstance().DispatchPacket(session, *packet);

        // ��Ŷ ��ȯ
        PacketPool::GetInstance().ReleasePacket(packet);
    }

    // ���� ����
    buffer->Clear();
}
