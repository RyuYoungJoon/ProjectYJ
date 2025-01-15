#pragma once
#include "MemoryPool.h"

// �� �� ��  ��  ��  �� �� �� �� �� ��
class MemoryPoolManager
{
public:
    // Ư�� ��� ũ���� MemoryPool�� ��ȯ
    static MemoryPool& GetMemoryPool(size_t blockSize)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // �̹� ������ MemoryPool�� �ִ� ��� ��ȯ
        auto it = m_Pools.find(blockSize);
        if (it != m_Pools.end())
        {
            return *(it->second);
        }

        // ������ ���� �����ϰ� ����
        auto pool = std::make_unique<MemoryPool>(blockSize);
        MemoryPool* poolPtr = pool.get();
        m_Pools[blockSize] = std::move(pool);
        return *poolPtr;
    }

private:
    // ��� ũ�⺰ MemoryPool ����
    static std::map<size_t, std::unique_ptr<MemoryPool>> m_Pools;
    static std::mutex m_Mutex;
};

//// static ��� �ʱ�ȭ
//