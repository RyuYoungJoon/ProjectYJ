#pragma once
#include "MemoryPool.h"

// ㅁ ㅁ ㅁ  ㅁ  ㅁ  ㅁ ㅁ ㅁ ㅁ ㅁ ㅁ
class MemoryPoolManager
{
public:
    // 특정 블록 크기의 MemoryPool을 반환
    static MemoryPool& GetMemoryPool(size_t blockSize)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // 이미 생성된 MemoryPool이 있는 경우 반환
        auto it = m_Pools.find(blockSize);
        if (it != m_Pools.end())
        {
            return *(it->second);
        }

        // 없으면 새로 생성하고 저장
        auto pool = std::make_unique<MemoryPool>(blockSize);
        MemoryPool* poolPtr = pool.get();
        m_Pools[blockSize] = std::move(pool);
        return *poolPtr;
    }

private:
    // 블록 크기별 MemoryPool 저장
    static std::map<size_t, std::unique_ptr<MemoryPool>> m_Pools;
    static std::mutex m_Mutex;
};

//// static 멤버 초기화
//