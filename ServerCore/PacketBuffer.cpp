#include "pch.h"
#include "PacketBuffer.h"

void PacketBuffer::Write(const void* data, std::size_t size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (WritableSize() < size)
    {
        throw std::overflow_error("Not enough space in buffer to write data.");
    }

    std::memcpy(&m_Buffer[m_WritePos], data, size);
    m_WritePos += size;
}

void PacketBuffer::Read(void* outData, std::size_t size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (ReadableSize() < size)
    {
        throw std::underflow_error("Not enough data in buffer to read.");
    }

    std::memcpy(outData, &m_Buffer[m_ReadPos], size);
    m_ReadPos += size;
}

void PacketBuffer::Peek(void* outData, std::size_t size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (ReadableSize() < size)
    {
        throw std::underflow_error("Not enough data in buffer to peek.");
    }

    std::memcpy(outData, &m_Buffer[m_ReadPos], size);
}

void PacketBuffer::DiscardReadData()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_ReadPos > 0)
    {
        std::size_t readable = ReadableSize();
        std::memmove(&m_Buffer[0], &m_Buffer[m_ReadPos], readable);
        m_ReadPos = 0;
        m_WritePos = readable;
    }
}

