#pragma once

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        // 객체 풀에 남아있는 모든 객체 해제
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            delete pool_.top();
            pool_.pop();
        }
    }

    // InitPool: 미리 count개의 객체를 생성하여 풀에 저장
    void InitPool(size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < count; ++i) {
            pool_.push(new T());
        }
    }

    // Pop: 풀에서 객체를 꺼내 shared_ptr로 반환
    std::shared_ptr<T> Pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        T* obj = nullptr;
        if (pool_.empty()) {
            // 풀에 남은 객체가 없으면 새로 생성
            LOGD << "Pool Size Expand! Pool Size : " << size();
            obj = new T();
        }
        else {
            obj = pool_.top();
            pool_.pop();
        }
        // custom deleter를 통해 shared_ptr가 소멸될 때 Push를 호출하여 객체를 풀로 복귀
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Push(ptr);
            });
    }

    // Push: 객체를 풀에 반환 (메모리 환원)
    void Push(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(obj);
    }

    // 현재 풀에 저장되어 있는 객체 수 반환
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

private:
    mutable std::mutex mutex_;
    std::stack<T*> pool_;  // push/pop 연산에 특화된 컨테이너
};