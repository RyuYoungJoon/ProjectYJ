#pragma once
#include <algorithm>

// 작업 큐에 저장될 Task의 우선순위 정의
enum class TaskPriority : uint8
{
    High = 0,
    Normal = 1,
    Low = 2
};

// Task 클래스 - 작업 큐에 들어갈 각 작업의 단위
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

    // 작업자 스레드 풀 초기화
    void Initialize(uint32 threadCount = 0)
    {
        if (threadCount == 0)
        {
            // 하드웨어 코어 수에 기반하여 스레드 수 결정
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

        // 작업자 스레드 생성
        for (uint32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this, i] {
                LOGI << "TaskQueue worker thread " << i << " started";
                this->WorkerThreadFunction();
                LOGI << "TaskQueue worker thread " << i << " stopped";
                });
        }
    }

    // 작업 큐 종료
    void Shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_Running) return;
            m_Running = false;
        }

        // 모든 대기중인 스레드를 깨움
        m_Condition.notify_all();

        // 모든 작업자 스레드 종료 대기
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }

        m_WorkerThreads.clear();

        // 남은 작업 비우기
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Tasks.empty())
            m_Tasks.pop();
    }

    // 새 작업 추가
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

    // 현재 대기 중인 작업 수 반환
    size_t GetPendingTaskCount()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Tasks.size();
    }

    // 작업자 스레드 수 반환
    size_t GetWorkerThreadCount()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_WorkerThreads.size();
    }

private:
    TaskQueue() : m_Running(false) {}

    // 작업자 스레드의 메인 루프
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
                
                // 종료 신호 확인
                if (!m_Running && m_Tasks.empty())
                    break;

                // 작업이 없으면 다시 대기
                if (m_Tasks.empty())
                    continue;

                // 작업 가져오기
                task = m_Tasks.top();
                m_Tasks.pop();
            }

            // 작업 실행
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
    // 우선순위 큐로 구현된 작업 큐
    std::priority_queue<std::shared_ptr<Task>> m_Tasks;
    std::vector<std::thread> m_WorkerThreads;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Running;
};
