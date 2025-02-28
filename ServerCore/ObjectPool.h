#pragma once

template <typename T>
class ObjectPool {
public:
    ObjectPool() = default;  // 생성자에서 초기화하지 않음

    void InitPool(std::size_t initialSize, std::size_t maxSize) {
        _maxSize = maxSize;
        _pool = std::make_unique<boost::object_pool<T>>(initialSize, initialSize / 2);
        PreAllocate(initialSize);
    }

    template <typename... Args>
    T* Allocate(Args&&... args) {
        if (_available.empty()) {
            if (_allocated.size() >= _maxSize) {
                std::cerr << "ObjectPool is full! Cannot allocate more.\n";
                return nullptr;
            }
            // 새 객체 생성
            T* obj = _pool->construct(std::forward<Args>(args)...);
            _allocated.push_back(obj);
            return obj;
        } else {
            // 기존 객체 재사용
            T* obj = _available.back();
            _available.pop_back();
            return obj;
        }
    }

    void Deallocate(T* obj) {
        if (obj) {
            _available.push_back(obj);  // 해제된 객체를 다시 풀로 반환
        }
    }

private:
    void PreAllocate(std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            T* obj = _pool->construct();
            _available.push_back(obj);
            _allocated.push_back(obj);
        }
    }

    std::size_t _maxSize = 0;
    std::unique_ptr<boost::object_pool<T>> _pool;
    std::vector<T*> _allocated;
    std::vector<T*> _available;
};