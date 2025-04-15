#pragma once
#include "SessionManager.h"

// ��Ʈ��ũ ���� ������
enum class NetState : uint8
{
    None,
    Connected,
    Disconnect,
    Reconnect
};

// I/O �۾� ����ü
struct IoTask
{
    int32 sessionUID;
    NetState state;

    IoTask() : sessionUID(-1), state(NetState::None) {}
    IoTask(int32 uid, NetState state) : sessionUID(uid), state(state) {}
};

// I/O �۾� ť
class TaskQueue
{
public:
    static TaskQueue& GetInstance()
    {
        static TaskQueue instance;
        return instance;
    }

    // �ʱ�ȭ
    void Initialize(int32 threadCount = 0)
    {
        if (m_IsRunning)
            return;

        m_IsRunning = true;

        // ������ �� ����
        if (threadCount <= 0)
        {
            threadCount = std::thread::hardware_concurrency() / 2;
            if (threadCount < 1)
                threadCount = 1;
        }

        // ��Ŀ ������ ����
        for (int32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this]() { WorkerThread(); });
        }

        LOGI << "IoTaskQueue initialized with " << threadCount << " worker threads";
    }

    // ����
    void Shutdown()
    {
        if (!m_IsRunning)
            return;

        m_IsRunning = false;

        // �� �۾��� ������ ����ŭ �߰��Ͽ� ��� ���� ��������� ����ǵ��� ��
        for (size_t i = 0; i < m_WorkerThreads.size(); ++i)
        {
            m_TaskQueue.push(IoTask());
        }

        // ��� ������ ���� ���
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }
        m_WorkerThreads.clear();

        // ť ����
        IoTask dummyTask;
        while (m_TaskQueue.try_pop(dummyTask)) {}

        LOGI << "IoTaskQueue shutdown complete";
    }

    // I/O �۾� �߰�
    void PushTask(int32 sessionUID, NetState state)
    {
        if (!m_IsRunning)
            return;

        IoTask task(sessionUID, state);
        m_TaskQueue.push(task);
    }

    // ���� ť ũ�� ��ȯ
    size_t GetQueueSize() const
    {
        return m_TaskQueue.unsafe_size();
    }

private:
    TaskQueue() : m_IsRunning(false) {}
    ~TaskQueue() { Shutdown(); }

    // ��Ŀ ������ �Լ�
    void WorkerThread()
    {
        IoTask task;

        while (m_IsRunning)
        {
            // �۾� ��������
            if (!m_TaskQueue.try_pop(task))
            {
                std::this_thread::sleep_for(100ms);
                continue;
            }

            // ���� �����͸� �� �ް� Disconnect �ؾ��Ѵ�.
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

    // I/O �۾� ó��
    void ProcessIoTask(const IoTask& task);

private:
    std::atomic<bool> m_IsRunning;
    Concurrency::concurrent_queue<IoTask> m_TaskQueue;
    std::vector<std::thread> m_WorkerThreads;
};