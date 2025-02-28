#pragma once

template <typename T>
class ObjectPool
{
public:
	ObjectPool();
	ObjectPool(size_t poolSize)
		: m_PoolSize(poolSize), m_ObjectQueue(poolSize)
	{}

	void InitPool(size_t poolSize)
	{
		m_PoolSize = poolSize;
		m_ObjectQueue.reserve(100);
	}

	std::shared_ptr<T> Pop()
	{
		std::shared_ptr<T> obj;
		if (m_ObjectQueue.pop(obj))
		{
			return obj;
		}

		return std::make_shared<T>();
	}

	void Push(std::shared_ptr<T> obj)
	{
		obj->Reset();
		m_ObjectQueue.push(obj);
	}

private:
	size_t m_PoolSize;
	boost::lockfree::stack<std::shared_ptr<T>> m_ObjectQueue;
};

template<typename T>
inline ObjectPool<T>::ObjectPool()
{
}
