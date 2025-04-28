#pragma once
#include <concurrent_unordered_map.h>

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        Clean();
    }

    // InitPool: �̸� count���� ��ü�� �����Ͽ� Ǯ�� ����
    void InitPool(size_t count) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_Pool.push(new T());
        }
    }

    // Pop: Ǯ���� ��ü�� ���� shared_ptr�� ��ȯ
    std::shared_ptr<T> Pop() 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        T* obj = nullptr;
        if (!m_Pool.empty()) {
            obj = m_Pool.top();
            m_Pool.pop();
        }
        else {
            // Ǯ�� ���� ��ü�� ������ ���� ����
            LOGD << "Pool Size Expand! Pool Size : " << GetSize();
            obj = new T();
        }

        // custom deleter�� ���� shared_ptr�� �Ҹ�� �� Push�� ȣ���Ͽ� ��ü�� Ǯ�� ����
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Push(ptr);
            });
    }

    // Push: ��ü�� Ǯ�� ��ȯ (�޸� ȯ��)
    void Push(T* obj) 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pool.push(obj);
    }

    // ���� Ǯ�� ����Ǿ� �ִ� ��ü �� ��ȯ
    size_t GetSize() const 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Pool.size();
    }

    void Clean()
    {
        // ��ü Ǯ�� �����ִ� ��� ��ü ����
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Pool.empty())
        {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

private:
    mutable std::mutex m_Mutex;
    std::stack<T*> m_Pool;  // push/pop ���꿡 Ưȭ�� �����̳�
};

// MemoryPool�� Ȱ���� Packet ��ü Ǯ
class PacketPool
{
public:
    static PacketPool& GetInstance()
    {
        static PacketPool instance;
        return instance;
    }

    // Ǯ �ʱ�ȭ
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

    // ��Ŷ �Ҵ�
    template<typename TPacket>
    TPacket* Pop()
    {
        auto typeIndex = std::type_index(typeid(TPacket));

        auto& pool = m_PacketPool[typeIndex];

        TPacket* packet = nullptr;
        void* temp = nullptr;

        // ��Ŷ ������
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

    // ��Ŷ ��ȯ
    template<typename TPacket>
    void Push(TPacket* packet)
    {
        if (!packet)
            return;

        auto typeIndex = std::type_index(typeid(TPacket));

        m_PacketPool[typeIndex].push(packet);

        m_ActiveCount.fetch_sub(1);
    }

    // Ǯ ���� (���ø����̼� ���� �� ȣ��)
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

    // Ȱ�� ��Ŷ �� ��ȯ
    size_t GetActiveCount() const
    {
        return m_ActiveCount.load();
    }

    // �� ��Ŷ �� ��ȯ
    size_t GetTotalCount() const
    {
        return m_TotalCount.load();
    }

    // ��� ������ ��Ŷ �� ��ȯ
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