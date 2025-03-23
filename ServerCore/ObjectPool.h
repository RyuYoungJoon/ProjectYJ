#pragma once
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
    void Init(size_t initialSize = 1000)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // 기존 메모리 정리
        while (!m_FreePackets.empty())
        {
            Packet* packet = m_FreePackets.top();
            m_FreePackets.pop();
            delete packet;
        }

        m_AllPackets.clear();

        // 새 패킷 할당
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

    // 패킷 할당
    Packet* Pop()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // 사용 가능한 패킷이 없으면 추가 할당
        if (m_FreePackets.empty())
        {
            // 현재 풀 크기의 절반만큼 확장
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

        // 패킷 초기화 (필요한 경우)
        memset(packet, 0, sizeof(Packet));

        m_ActiveCount++;
        return packet;
    }

    // 패킷 반환
    void Push(Packet* packet)
    {
        if (!packet) return;

        std::lock_guard<std::mutex> lock(m_Mutex);

        // 유효한 패킷인지 확인 (선택적)
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

    // 풀 정리 (애플리케이션 종료 시 호출)
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

    // 활성 패킷 수 반환
    size_t GetActiveCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_ActiveCount;
    }

    // 총 패킷 수 반환
    size_t GetTotalCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_AllPackets.size();
    }

    // 사용 가능한 패킷 수 반환
    size_t GetAvailableCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_FreePackets.size();
    }

private:
    PacketPool() : m_ActiveCount(0) {}
    ~PacketPool() { Clean(); }

    mutable std::mutex m_Mutex;
    std::vector<Packet*> m_AllPackets;    // 모든 할당된 패킷 추적
    std::stack<Packet*> m_FreePackets;    // 사용 가능한 패킷 스택
    size_t m_ActiveCount;                 // 현재 활성화된 패킷 수
};