#pragma once
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
    void Init(size_t initialSize = 1000)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // ���� �޸� ����
        while (!m_FreePackets.empty())
        {
            Packet* packet = m_FreePackets.top();
            m_FreePackets.pop();
            delete packet;
        }

        m_AllPackets.clear();

        // �� ��Ŷ �Ҵ�
        for (size_t i = 0; i < initialSize; ++i)
        {
            Packet* packet = new Packet();
            memset(packet, 0, sizeof(Packet));

            m_AllPackets.push_back(packet);
            m_FreePackets.push(packet);
        }

        m_ActiveCount = 0;
        LOGI << "PacketPool initialized with " << initialSize << " packets";
    }

    // ��Ŷ �Ҵ�
    Packet* Pop()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // ��� ������ ��Ŷ�� ������ �߰� �Ҵ�
        if (m_FreePackets.empty())
        {
            // ���� Ǯ ũ���� ���ݸ�ŭ Ȯ��
            size_t expandSize = m_AllPackets.size() / 2;

            for (size_t i = 0; i < expandSize; ++i)
            {
                Packet* packet = new Packet();
                memset(packet, 0, sizeof(Packet));

                m_AllPackets.push_back(packet);
                m_FreePackets.push(packet);
            }

            LOGI << "PacketPool expanded with " << expandSize << " additional packets";
        }

        Packet* packet = m_FreePackets.top();
        m_FreePackets.pop();

        // ��Ŷ �ʱ�ȭ (�ʿ��� ���)
        memset(packet, 0, sizeof(Packet));

        m_ActiveCount++;
        return packet;
    }

    // ��Ŷ ��ȯ
    void Push(Packet* packet)
    {
        if (!packet) return;

        std::lock_guard<std::mutex> lock(m_Mutex);

        // ��ȿ�� ��Ŷ���� Ȯ�� (������)
        bool validPacket = false;
        for (auto p : m_AllPackets)
        {
            if (p == packet)
            {
                validPacket = true;
                break;
            }
        }

        if (!validPacket)
        {
            LOGW << "Attempted to return an invalid packet to the pool";
            return;
        }

        m_FreePackets.push(packet);
        m_ActiveCount--;
    }

    // Ǯ ���� (���ø����̼� ���� �� ȣ��)
    void Clean()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (auto packet : m_AllPackets)
        {
            delete packet;
        }

        m_AllPackets.clear();

        while (!m_FreePackets.empty())
        {
            m_FreePackets.pop();
        }

        m_ActiveCount = 0;
    }

    // Ȱ�� ��Ŷ �� ��ȯ
    size_t GetActiveCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_ActiveCount;
    }

    // �� ��Ŷ �� ��ȯ
    size_t GetTotalCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_AllPackets.size();
    }

    // ��� ������ ��Ŷ �� ��ȯ
    size_t GetAvailableCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_FreePackets.size();
    }

private:
    PacketPool() : m_ActiveCount(0) {}
    ~PacketPool() { Clean(); }

    mutable std::mutex m_Mutex;
    std::vector<Packet*> m_AllPackets;    // ��� �Ҵ�� ��Ŷ ����
    std::stack<Packet*> m_FreePackets;    // ��� ������ ��Ŷ ����
    size_t m_ActiveCount;                 // ���� Ȱ��ȭ�� ��Ŷ ��
};