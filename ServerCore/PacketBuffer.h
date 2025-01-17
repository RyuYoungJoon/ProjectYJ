#pragma once
#include "MemoryPoolManager.h"


class PacketBuffer
{
public:
    PacketBuffer(std::size_t bufferSize)
        : m_Buffer(nullptr), m_BufferSize(bufferSize), m_ReadPos(0), m_WritePos(0)
    {
        m_Buffer = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(bufferSize).Allocate());
    }

    ~PacketBuffer() = default;

    
    void Write(const void* data, std::size_t size);
    void Read(void* outData, std::size_t size);
    void Peek(void* outData, std::size_t size);
    void DiscardReadData();

    std::size_t ReadableSize() const { return m_WritePos - m_ReadPos; }
    std::size_t WritableSize() const { return m_BufferSize - m_WritePos; }

    void Clear()
    {
        m_ReadPos = 0;
        m_WritePos = 0;
    }

private:
    BYTE* m_Buffer;
    size_t m_BufferSize;
    std::mutex m_Mutex;
    std::size_t m_ReadPos;
    std::size_t m_WritePos;
};
