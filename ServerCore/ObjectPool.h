#pragma once

template <typename T>
class ObjectPool {
public:
    ObjectPool() {};

    ~ObjectPool() {
        // ��ü Ǯ�� �����ִ� ��� ��ü ����
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            delete pool_.top();
            pool_.pop();
        }
    }

    // InitPool: �̸� count���� ��ü�� �����Ͽ� Ǯ�� ����
    void InitPool(size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < count; ++i) {
            pool_.push(new T());
        }
    }

    // Pop: Ǯ���� ��ü�� ���� shared_ptr�� ��ȯ
    std::shared_ptr<T> Pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        T* obj = nullptr;
        if (pool_.empty()) {
            // Ǯ�� ���� ��ü�� ������ ���� ����
            LOGD << "Pool Size Expand! Pool Size : " << size();
            obj = new T();
        }
        else {
            obj = pool_.top();
            pool_.pop();
        }
        // custom deleter�� ���� shared_ptr�� �Ҹ�� �� Push�� ȣ���Ͽ� ��ü�� Ǯ�� ����
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Push(ptr);
            });
    }

    // Push: ��ü�� Ǯ�� ��ȯ (�޸� ȯ��)
    void Push(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(obj);
    }

    // ���� Ǯ�� ����Ǿ� �ִ� ��ü �� ��ȯ
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

private:
    mutable std::mutex mutex_;
    std::stack<T*> pool_;  // push/pop ���꿡 Ưȭ�� �����̳�
};