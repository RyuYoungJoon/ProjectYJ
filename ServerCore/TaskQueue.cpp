#include "pch.h"
#include "TaskQueue.h"

void TaskQueue::ProcessTask()
{
	while (true)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Condition.wait(lock, [this]() {
				return !m_Tasks.empty();
				});
			task = std::move(m_Tasks.front());
			m_Tasks.pop();
		}

		task();
	}
}
