#pragma once

// [r][][][w][][][][][][][][][]
class PacketBuffer
{
public:
    PacketBuffer(int32 bufferSize);
    ~PacketBuffer() = default;

    bool OnWrite(size_t size);
    bool OnRead(int32 size);
    void Clear();

    BYTE* ReadPos() { return &m_Buffer[m_ReadPos]; }
    BYTE* WritePos() { return &m_Buffer[m_WritePos]; }
    int32 DataSize() const { return m_WritePos - m_ReadPos; }
    int32 FreeSize() const { return m_Capacity - m_WritePos; }

private:
    vector<BYTE> m_Buffer;
    int32 m_Capacity = 0;
    int32 m_BufferSize = 0;
    int32 m_ReadPos = 0;
    int32 m_WritePos = 0;
};
