#pragma once
#include <concurrent_unordered_map.h>

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        Clean();
    }

    // InitPool: 미리 count개의 객체를 생성하여 풀에 저장
    void InitPool(size_t count) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_Pool.push(new T());
        }
    }

    // Pop: 풀에서 객체를 꺼내 shared_ptr로 반환
    std::shared_ptr<T> Pop() 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        T* obj = nullptr;
        if (!m_Pool.empty()) {
            obj = m_Pool.top();
            m_Pool.pop();
        }
        else {
            // 풀에 남은 객체가 없으면 새로 생성
            LOGD << "Pool Size Expand! Pool Size : " << GetSize();
            obj = new T();
        }

        // custom deleter를 통해 shared_ptr가 소멸될 때 Push를 호출하여 객체를 풀로 복귀
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Push(ptr);
            });
    }

    // Push: 객체를 풀에 반환 (메모리 환원)
    void Push(T* obj) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pool.push(obj);
    }

    // 현재 풀에 저장되어 있는 객체 수 반환
    size_t GetSize() const 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Pool.size();
    }

    void Clean()
    {
        // 객체 풀에 남아있는 모든 객체 해제
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Pool.empty())
        {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

private:
    mutable std::mutex m_Mutex;
    std::stack<T*> m_Pool;  // push/pop 연산에 특화된 컨테이너
};

// MemoryPool을 활용한 Packet 객체 풀
class PacketPool
{
public:
    static PacketPool& GetInstance()
    {
        static PacketPool instance;
        return instance;
    }

    // 풀 초기화
    template<typename TPacket>
    void Init(size_t initialSize = 1000)
    {
        Clean();

        m_PoolSize = initialSize;

        auto typeIndex = std::type_index(typeid(TPacket));
        auto& pool = m_PacketPool[typeIndex];

        for (int i = 0; i < m_PoolSize; ++i)
        {
            TPacket* packet = new TPacket();
            pool.push(packet);
            m_TotalCount.fetch_add(1);
        }
    }

    // 패킷 할당
    template<typename TPacket>
    TPacket* Pop()
    {
        auto typeIndex = std::type_index(typeid(TPacket));

        auto& pool = m_PacketPool[typeIndex];

        TPacket* packet = nullptr;
        void* temp = nullptr;

        // 패킷 꺼내기
        if (!pool.try_pop(temp))
        {
            packet = new TPacket();
            m_TotalCount.fetch_add(1);
        }
        else
        {
            packet = static_cast<TPacket*>(temp);
        }

        memset(packet, 0, sizeof(TPacket));
        new(packet)TPacket();

        m_ActiveCount.fetch_add(1);
        return packet;
    }

    // 패킷 반환
    template<typename TPacket>
    void Push(TPacket* packet)
    {
        if (!packet)
            return;

        auto typeIndex = std::type_index(typeid(TPacket));

        m_PacketPool[typeIndex].push(packet);

        m_ActiveCount.fetch_sub(1);
    }

    // 풀 정리 (애플리케이션 종료 시 호출)
    void Clean()
    {
        std::lock_guard<std::mutex> lock(m_CleanMutex);

        auto tempMap = m_PacketPool;

        for (auto& pair : tempMap)
        {
            auto& packetQueue = pair.second;
            void* obj = nullptr;
            while (packetQueue.try_pop(obj))
            {
                delete obj;
            }
        }

        m_PacketPool.clear();
        m_ActiveCount.store(0);
        m_TotalCount.store(0);
    }

    // 활성 패킷 수 반환
    size_t GetActiveCount() const
    {
        return m_ActiveCount.load();
    }

    // 총 패킷 수 반환
    size_t GetTotalCount() const
    {
        return m_TotalCount.load();
    }

    // 사용 가능한 패킷 수 반환
    size_t GetAvailableCount() const
    {
        return m_TotalCount.load() - m_ActiveCount.load();
    }

private:
    PacketPool() : m_PoolSize(1000) {}
    ~PacketPool() { Clean(); }

    using PacketQueue = concurrency::concurrent_queue<void*>;
    concurrency::concurrent_unordered_map<std::type_index, PacketQueue> m_PacketPool;

    std::mutex m_CleanMutex;
    std::atomic<size_t> m_ActiveCount{ 0 };
    std::atomic<size_t> m_TotalCount{ 0 };
    size_t m_PoolSize;
};