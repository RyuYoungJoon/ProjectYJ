#pragma once
#include "MemoryPoolManager.h"

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        // ��ü Ǯ�� �����ִ� ��� ��ü ����
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Pool.empty()) {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

    // InitPool: �̸� count���� ��ü�� �����Ͽ� Ǯ�� ����
    void InitPool(size_t count) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_Pool.push(new T());
        }
    }

    // Pop: Ǯ���� ��ü�� ���� shared_ptr�� ��ȯ
    std::shared_ptr<T> Pop() {
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
    void Push(T* obj) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pool.push(obj);
    }

    // ���� Ǯ�� ����Ǿ� �ִ� ��ü �� ��ȯ
    size_t GetSize() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Pool.size();
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

    // Packet ��ü ��������
    Packet* Pop()
    {
        // MemoryPoolManager���� Packet ũ�⸸ŭ�� �޸� �Ҵ�
        BYTE* memory = static_cast<BYTE*>(MemoryPoolManager::GetMemoryPool(sizeof(Packet)).Allocate());

        // �޸� �ʱ�ȭ
        memset(memory, 0, sizeof(Packet));

        // BYTE* �����͸� Packet* �����ͷ� ��ȯ
        return reinterpret_cast<Packet*>(memory);
    }

    // Packet ��ü ��ȯ
    void Push(Packet* packet)
    {
        if (!packet)
            return;

        // Packet �����͸� BYTE* �����ͷ� ��ȯ �� MemoryPool�� ��ȯ
        MemoryPoolManager::GetMemoryPool(sizeof(Packet)).Deallocate(reinterpret_cast<void*>(packet));
    }

    // ��Ŷ ũ�⿡ �´� �޸� Ǯ ���� �ʱ�ȭ (������)
    void InitPool(size_t initialSize = 1000)
    {
        // �̹� MemoryPoolManager�� ���������� Ǯ�� �����ϹǷ� 
        // ���⼭�� �α׸� ���
        LOGI << "PacketPool initialized using MemoryPoolManager";
    }

private:
    PacketPool() {}
    ~PacketPool() {}
};