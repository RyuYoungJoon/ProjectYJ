#pragma once

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