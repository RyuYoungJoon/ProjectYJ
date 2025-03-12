#pragma once
#include "MemoryPoolManager.h"

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        // 객체 풀에 남아있는 모든 객체 해제
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Pool.empty()) {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

    // InitPool: 미리 count개의 객체를 생성하여 풀에 저장
    void InitPool(size_t count) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_Pool.push(new T());
        }
    }

    // Pop: 풀에서 객체를 꺼내 shared_ptr로 반환
    std::shared_ptr<T> Pop() {
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
    void Push(T* obj) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pool.push(obj);
    }

    // 현재 풀에 저장되어 있는 객체 수 반환
    size_t GetSize() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Pool.size();
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

    // Packet 객체 가져오기
    Packet* Pop()
    {
        // MemoryPoolManager에서 Packet 크기만큼의 메모리 할당
        BYTE* memory = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(sizeof(Packet)).Allocate());

        // 메모리 초기화
        memset(memory, 0, sizeof(Packet));

        // BYTE* 포인터를 Packet* 포인터로 변환
        return reinterpret_cast<Packet*>(memory);
    }

    // Packet 객체 반환
    void Push(Packet* packet)
    {
        if (!packet)
            return;

        // Packet 포인터를 BYTE* 포인터로 변환 후 MemoryPool에 반환
        MemoryPoolManager::GetMemoryPool(sizeof(Packet)).Deallocate(reinterpret_cast<void*>(packet));
    }

    // 패킷 크기에 맞는 메모리 풀 사전 초기화 (선택적)
    void InitPool(size_t initialSize = 1000)
    {
        // 이미 MemoryPoolManager가 내부적으로 풀을 관리하므로 
        // 여기서는 로그만 출력
        LOGI << "PacketPool initialized using MemoryPoolManager";
    }

private:
    PacketPool() {}
    ~PacketPool() {}
};