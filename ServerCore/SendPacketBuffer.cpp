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

void SendPacketBuffer::ProcessSessionBuffer(AsioSessionPtr session, size_t length)
{
    if (!session)
        return;

    int32 sessionId = session->GetSessionUID();

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_SessionBuffers.find(sessionId);
    if (it == m_SessionBuffers.end())
        return;

    // ��Ŷ ó��
    //while (buffer->DataSize() >= sizeof(PacketHeader))
    //{
    //    // ����� ���� Ȯ��
    //    PacketHeader header;
    //    memcpy(&header, buffer->ReadPos(), sizeof(PacketHeader));

    //    // ��Ŷ �ϼ� üũ
    //    if (buffer->DataSize() < header.size)
    //        break;

    //    // ��Ŷ Ǯ���� ��Ŷ ��������
    //    shared_ptr<Packet> packet = m_PacketPool.Pop();
    //    memcpy(&packet->header, buffer->ReadPos(), sizeof(PacketHeader));

    //    if (header.size > sizeof(PacketHeader))
    //        memcpy(packet->payload, buffer->ReadPos() + sizeof(PacketHeader), header.size - sizeof(PacketHeader));

    //    // Ǯ���� ������ Packet���� ���� �ƹ� ���� ���µ�
    //    // �� packet ���� buffer�� �ִ� ���ǹ��� �۾�.
    //    // packet���� �ƹ� ���� ���µ� buffer�� ���� ��Ŷ ����ͷ� Dispatch�ϴ���.
    //    // ���� �� �ʿ�.
    //    // 
    //    // ��Ŷ ������ ����
    //    
    //    // ��Ŷ ����ó�� ����
    //    PacketRouter::GetInstance().Dispatch(session, *packet);
    //    
    //    buffer->OnRead(header.size);
    //    buffer->Clear();

    //    // ��Ŷ Ǯ ��ȯ�� ����Ʈ �������̱� ������ �ڵ� Push�ȴ�.
    //}

    //int32 processLen = 0;
    //while (true)
    //{
    //    int32 dataSize = length - processLen;
    //    if (dataSize < sizeof(PacketHeader))
    //        break;

    //    PacketHeader header = 
    //}

    // ���� ����
    PacketBuffer* buffer = it->second.get();

    int32 processLen = session->ProcessPacket(buffer->ReadPos(), buffer->DataSize());

    if (processLen > 0)
    {
        buffer->OnRead(processLen);
    }

    buffer->Clear();

}
