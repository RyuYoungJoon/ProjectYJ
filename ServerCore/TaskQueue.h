#pragma once
#include <algorithm>

// �۾� ť�� ����� Task�� �켱���� ����
enum class TaskPriority : uint8
{
    High = 0,
    Normal = 1,
    Low = 2
};

// Task Ŭ���� - �۾� ť�� �� �� �۾��� ����
class Task
{
public:
    Task(std::function<void()> func)
        : m_Function(std::move(func)) {
    }

    void Execute()
    {
        if (m_Function)
            m_Function();
    }

private:
    std::function<void()> m_Function;
};


class TaskQueue
{
public:
    ~TaskQueue()
    {
        Shutdown();
    }

    static TaskQueue& GetInstance()
    {
        static TaskQueue instance;
        return instance;
    }

    // �۾��� ������ Ǯ �ʱ�ȭ
    void Initialize(uint32 threadCount = 0)
    {
        if (threadCount == 0)
        {
            // �ϵ���� �ھ� ���� ����Ͽ� ������ �� ����
            threadCount = std::thread::hardware_concurrency() / 2;
            if (threadCount < 1)
                threadCount = 1;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        if (!m_WorkerThreads.empty())
        {
            LOGE << "TaskQueue already initialized!";
            return;
        }

        m_Running = true;
        LOGI << "Starting TaskQueue with " << threadCount << " worker threads";

        // �۾��� ������ ����
        for (uint32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this, i] {
                LOGI << "TaskQueue worker thread " << i << " started";
                this->WorkerThreadFunction();
                LOGI << "TaskQueue worker thread " << i << " stopped";
                });
        }
    }

    // �۾� ť ����
    void Shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_Running) return;
            m_Running = false;
        }

        // ��� ������� �����带 ����
        m_Condition.notify_all();

        // ��� �۾��� ������ ���� ���
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }

        m_WorkerThreads.clear();

        // ���� �۾� ����
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Tasks.empty())
            m_Tasks.pop();
    }

    // �� �۾� �߰�
    void PushTask(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_Running)
            {
                LOGW << "Trying to push task to stopped TaskQueue";
                return;
            }

            auto taskPtr = std::make_shared<Task>(std::move(task));
            m_Tasks.push(taskPtr);
        }
        m_Condition.notify_all();
    }

    // ���� ��� ���� �۾� �� ��ȯ
    size_t GetPendingTaskCount()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Tasks.size();
    }

    // �۾��� ������ �� ��ȯ
    size_t GetWorkerThreadCount()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_WorkerThreads.size();
    }

private:
    TaskQueue() : m_Running(false) {}

    // �۾��� �������� ���� ����
    void WorkerThreadFunction()
    {
        while (true)
        {
            std::shared_ptr<Task> task;
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                if (m_Tasks.empty())
                {
                    m_Condition.wait(lock, [this] {
                        return !m_Running || !m_Tasks.empty();
                        });
                }
                
                // ���� ��ȣ Ȯ��
                if (!m_Running && m_Tasks.empty())
                    break;

                // �۾��� ������ �ٽ� ���
                if (m_Tasks.empty())
                    continue;

                // �۾� ��������
                task = m_Tasks.top();
                m_Tasks.pop();
            }

            // �۾� ����
            try
            {
                task->Execute();
            }
            catch (const std::exception& e)
            {
                LOGE << "Exception in task execution: " << e.what();
            }
            catch (...)
            {
                LOGE << "Unknown exception in task execution";
            }
        }
    }

private:
    // �켱���� ť�� ������ �۾� ť
    std::priority_queue<std::shared_ptr<Task>> m_Tasks;
    std::vector<std::thread> m_WorkerThreads;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Running;
};
