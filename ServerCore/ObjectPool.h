#pragma once

template <typename T>
class ObjectPool
{
public:
	ObjectPool() = default;
	ObjectPool(size_t poolSize)
		: m_PoolSize(poolSize), m_ObjectStack(poolSize)
	{}

	void InitPool(size_t poolSize)
	{
		m_PoolSize = poolSize;
		m_ObjectStack = std::make_unique<boost::lockfree::stack<std::shared_ptr<T>>>(poolSize);
	}

	std::shared_ptr<T> Pop()
	{
		std::shared_ptr<T> obj;
		if (m_ObjectStack.pop(obj))
		{
			return obj;
		}

		return std::make_shared<T>();
	}

	void Push(std::shared_ptr<T> obj)
	{
		obj->Reset();
		m_ObjectStack.push(obj);
	}

private:
	size_t m_PoolSize;
	boost::lockfree::stack<std::shared_ptr<T>> m_ObjectStack;
	std::mutex m_Mutex;
};