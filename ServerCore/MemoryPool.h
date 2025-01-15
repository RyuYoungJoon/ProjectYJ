#pragma once
class MemoryPool
{
public:
    explicit MemoryPool(size_t blockSize, size_t initialBlocks = 32)
        : m_BlockSize(blockSize)
    {
        ExpandPool(initialBlocks);
    }

    ~MemoryPool()
    {
        for (void* block : m_Blocks)
        {
            ::operator delete(block);
        }
    }

    void* Allocate()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_FreeBlocks.empty())
        {
            ExpandPool(m_Blocks.size());
        }

        void* block = m_FreeBlocks.front();
        m_FreeBlocks.pop();
        return block;
    }

    void Deallocate(void* block)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_FreeBlocks.push(block);
    }

private:
    void ExpandPool(size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            void* block = ::operator new(m_BlockSize);
            m_Blocks.push_back(block);
            m_FreeBlocks.push(block);
        }
    }

    size_t m_BlockSize;
    std::vector<void*> m_Blocks;
    std::queue<void*> m_FreeBlocks;
    std::mutex m_Mutex;
};