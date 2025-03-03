#pragma once

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