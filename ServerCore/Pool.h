#pragma once
#include <concurrent_unordered_map.h>
#include "Packet.h"

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        Clean();
    }

    // InitPool
    void InitPool(size_t count) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_Pool.push(new T());
        }
    }

    // Pop
    std::shared_ptr<T> Pop() 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        T* obj = nullptr;
        if (!m_Pool.empty()) {
            obj = m_Pool.top();
            m_Pool.pop();
        }
        else {
            // 
            LOGD << "Pool Size Expand! Pool Size : " << GetSize();
            obj = new T();
        }

        // custom deleter
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Push(ptr);
            });
    }

    // Push
    void Push(T* obj) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pool.push(obj);
    }

    size_t GetSize() const 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Pool.size();
    }

    void Clean()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Pool.empty())
        {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

private:
    mutable std::mutex m_Mutex;
    std::stack<T*> m_Pool;  // push/pop
};

class PacketPool
{
public:
    using PacketPtr = std::shared_ptr<Packet>;

private:

    //boost::lockfree::queue<PacketPtr*> m_PacketPool;
    Concurrency::concurrent_queue<PacketPtr> m_PacketPool;

    std::atomic<size_t> m_UseConut{ 0 };
    std::atomic<size_t> m_TotalCount{ 0 };
    std::atomic<size_t> m_AvailableCount{ 0 };

    uint32 m_PoolSize;

public:
    PacketPool() {}
    ~PacketPool() { Clear(); }

    static PacketPool& GetInstance()
    {
        static PacketPool instance;
        return instance;
    }

    bool Init(uint32 poolSize)
    {
        m_PoolSize = poolSize;

        for (uint32 i = 0; i < poolSize; ++i)
        {
            auto packet = std::make_shared<Packet>();
            m_PacketPool.push(packet);
            m_TotalCount.fetch_add(1, std::memory_order_relaxed);
            m_AvailableCount.fetch_add(1, std::memory_order_relaxed);
        }

        LOGI << "PacketPool Init! TotalCount : " << m_TotalCount << " Packet";
        return m_TotalCount > 0;
    }

    PacketPtr Pop()
    {
        PacketPtr packet;

        if (m_PacketPool.try_pop(packet))
        {
            //packet->Reset();
            m_UseConut.fetch_add(1, std::memory_order_relaxed);
            m_AvailableCount.fetch_sub(1, std::memory_order_relaxed);
            return packet;
        }
        else
        {
            packet = std::make_shared<Packet>();
            m_TotalCount.fetch_add(1, std::memory_order_relaxed);
            m_UseConut.fetch_add(1, std::memory_order_relaxed);

            LOGW << "PacketPool Create New Packet" << m_TotalCount;
            return packet;
        }
    }

    void Push(PacketPtr packet)
    {
        if (!packet)
            return;
       // packet->Reset();

        if (m_AvailableCount.load(std::memory_order_relaxed) < m_PoolSize)
        {
            m_PacketPool.push(packet);
            
            m_AvailableCount.fetch_sub(1, std::memory_order_relaxed);
            
        }

        m_UseConut.fetch_sub(1, std::memory_order_relaxed);
    }

    void Clear()
    {
        PacketPtr packet;
        int32 count = 0;
        
        while (m_PacketPool.try_pop(packet))
        {
            count++;
        }

        m_TotalCount.store(0, std::memory_order_relaxed);
        m_UseConut.store(0, std::memory_order_relaxed);
        m_AvailableCount.store(0, std::memory_order_relaxed);

        LOGI << "PacketPool Clear Complete " << count << " Packet";
    }
};