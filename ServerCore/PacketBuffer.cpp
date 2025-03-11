#include "pch.h"
#include "PacketBuffer.h"


PacketBuffer::PacketBuffer()
{
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

bool PacketBuffer::OnWrite(const BYTE* data, int32 size)
{
    if (FreeSize() < size)
        return false;

    memcpy(&m_Buffer[m_WritePos], data, size);
    m_WritePos += size;
    return true;
}

bool PacketBuffer::OnRead(BYTE* data, int32 size)
{
    if (DataSize() < size)
        return false;

    memcpy(data, &m_Buffer[m_ReadPos], size);
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

