#pragma once

class PacketBuffer
{
public:
    PacketBuffer(int32 bufferSize);
    ~PacketBuffer() = default;

    bool OnWrite(int32 size);
    bool OnRead(int32 size);
    void DiscardReadData();

    BYTE* ReadPos() { return &m_Buffer[m_ReadPos]; }
    BYTE* WritePos() { return &m_Buffer[m_WritePos]; }
    int32 DataSize() const { return m_WritePos - m_ReadPos; }
    int32 FreeSize() const { return m_Capacity - m_WritePos; }

    void Clear()
    {
        m_ReadPos = 0;
        m_WritePos = 0;
    }

private:
    vector<BYTE> m_Buffer;
    int32 m_Capacity = 0;
    int32 m_BufferSize = 0;
    int32 m_ReadPos = 0;
    int32 m_WritePos = 0;
};
