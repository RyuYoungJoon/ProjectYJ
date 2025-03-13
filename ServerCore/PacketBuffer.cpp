#include "pch.h"
#include "PacketBuffer.h"


PacketBuffer::PacketBuffer()
{
    Init(65536);
}

PacketBuffer::PacketBuffer(int32 bufferSize)
    : m_BufferSize(bufferSize)
{
    m_Capacity = bufferSize * 10;
    m_Buffer.resize(m_Capacity);
}

void PacketBuffer::Init(int32 bufferSize)
{
    m_BufferSize = bufferSize;
    m_Capacity = bufferSize * 10;
    m_Buffer.resize(m_Capacity);
}

bool PacketBuffer::OnWrite(const BYTE* buffer, int32 size)
{
    if (FreeSize() < size)
        return false;

    memcpy(&m_Buffer[m_WritePos], buffer, size);
    m_WritePos += size;
    return true;
}

bool PacketBuffer::OnRead(BYTE* buffer ,int32 size)
{
    if (DataSize() < size)
        return false;

    memcpy(&m_Buffer[m_WritePos], buffer, size);
    m_ReadPos += size;
    return true;}

bool PacketBuffer::OnWrite(int32 size)
{
    if (FreeSize() < size)
        return false;

    m_WritePos += size;
    return true;
}

bool PacketBuffer::OnRead(int32 size)
{
    if (DataSize() < size)
        return false;

    m_ReadPos += size;
    return true;
}

void PacketBuffer::Clear()
{
    int32 dataSize = DataSize();
    if (dataSize == 0)
    {
        // �� ��ħ �б�+���� Ŀ���� ������ ��ġ���, �� �� ����.
        m_ReadPos = m_WritePos = 0;
    }
    else
    {
        // ���� ������ ���� 1�� ũ�� �̸��̸�, �����͸� ������ �����.
        if (FreeSize() < m_BufferSize)
        {
            ::memcpy(&m_Buffer[0], &m_Buffer[m_ReadPos], dataSize);
            m_ReadPos = 0;
            m_WritePos = dataSize;
        }
    }
}

