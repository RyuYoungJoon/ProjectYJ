#pragma once
#include "SessionManager.h"

// 네트워크 상태 열거형
enum class NetState : uint8
{
    None,
    Connected,
    Disconnect,
    Reconnect
};

// I/O 작업 구조체
struct IoTask
{
    int32 sessionUID;
    NetState state;

    IoTask() : sessionUID(-1), state(NetState::None) {}
    IoTask(int32 uid, NetState state) : sessionUID(uid), state(state) {}
};

// I/O 작업 큐
class TaskQueue
{
public:
    static TaskQueue& GetInstance()
    {
        static TaskQueue instance;
        return instance;
    }

    // 초기화
    void Initialize(int32 threadCount = 0)
    {
        if (m_IsRunning)
            return;

        m_IsRunning = true;

        // 스레드 수 결정
        if (threadCount <= 0)
        {
            threadCount = std::thread::hardware_concurrency() / 2;
            if (threadCount < 1)
                threadCount = 1;
        }

        // 워커 스레드 생성
        for (int32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this]() { WorkerThread(); });
        }

        LOGI << "IoTaskQueue initialized with " << threadCount << " worker threads";
    }

    // 종료
    void Shutdown()
    {
        if (!m_IsRunning)
            return;

        m_IsRunning = false;

        // 빈 작업을 스레드 수만큼 추가하여 대기 중인 스레드들이 종료되도록 함
        for (size_t i = 0; i < m_WorkerThreads.size(); ++i)
        {
            m_TaskQueue.push(IoTask());
        }

        // 모든 스레드 종료 대기
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }
        m_WorkerThreads.clear();

        // 큐 비우기
        IoTask dummyTask;
        while (m_TaskQueue.try_pop(dummyTask)) {}

        LOGI << "IoTaskQueue shutdown complete";
    }

    // I/O 작업 추가
    void PushTask(int32 sessionUID, NetState state)
    {
        if (!m_IsRunning)
            return;

        IoTask task(sessionUID, state);
        m_TaskQueue.push(task);
    }

    // 현재 큐 크기 반환
    size_t GetQueueSize() const
    {
        return m_TaskQueue.unsafe_size();
    }

private:
    TaskQueue() : m_IsRunning(false) {}
    ~TaskQueue() { Shutdown(); }

    // 워커 스레드 함수
    void WorkerThread()
    {
        IoTask task;

        while (m_IsRunning)
        {
            // 작업 가져오기
            if (!m_TaskQueue.try_pop(task))
            {
                std::this_thread::sleep_for(100ms);
                continue;
            }

            // 받은 데이터를 다 받고 Disconnect 해야한다.
            //AsioSessionPtr session = SessionManager::GetInstance().GetSession(task.sessionUID);
            //if (session->GetSessionState() != SessionState::Recv)
            //    continue;
            
            try
            {
                ProcessIoTask(task);
            }
            catch (const std::exception& e)
            {
                LOGE << "Exception in worker thread: " << e.what();
            }
            catch (...)
            {
                LOGE << "Unknown exception in worker thread";
            }
        }
    }

    // I/O 작업 처리
    void ProcessIoTask(const IoTask& task);

private:
    std::atomic<bool> m_IsRunning;
    Concurrency::concurrent_queue<IoTask> m_TaskQueue;
    std::vector<std::thread> m_WorkerThreads;
};