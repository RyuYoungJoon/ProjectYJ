#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

class PacketBuffer
{
public:
    PacketBuffer(std::size_t bufferSize = 4096)
        : m_Buffer(bufferSize), m_ReadPos(0), m_WritePos(0)
    {
    }

    ~PacketBuffer() = default;

    
    void Write(const void* data, std::size_t size);
    void Read(void* outData, std::size_t size);
    void Peek(void* outData, std::size_t size);
    void DiscardReadData();

    std::size_t ReadableSize() const { return m_WritePos - m_ReadPos; }
    std::size_t WritableSize() const { return m_Buffer.size() - m_WritePos; }

    void Clear()
    {
        m_ReadPos = 0;
        m_WritePos = 0;
    }

    std::vector<uint8_t> m_Buffer;
private:
    std::mutex m_Mutex;
    std::size_t m_ReadPos;
    std::size_t m_WritePos;
};
