#pragma once
class TaskQueue
{
public:
	static TaskQueue& GetInstance()
	{
		static TaskQueue instance;
		return instance;
	}

	void PushTask(std::function<void()> task)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Tasks.push(std::move(task));
		m_Condition.notify_one();
	}

	void ProcessTask();

private:
	std::queue<std::function<void()>> m_Tasks;
	std::mutex m_Mutex;
	std::condition_variable m_Condition;
};

