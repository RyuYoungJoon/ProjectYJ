#pragma once
#include <algorithm>
#include "AsioSession.h"

// �۾� ť�� ����� Task�� �켱���� ����
// ��Ŷ ó���� ���� �۾� ����
struct PacketTask
{
    AsioSessionPtr session;
    std::vector<BYTE> buffer;
    size_t length;
};

class PacketQueue
{
public:
    static PacketQueue& GetInstance()
    {
        static PacketQueue instance;
        return instance;
    }

    // ��Ŀ ������ Ǯ �ʱ�ȭ
    void Initialize(int32 threadCount = 0)
    {
        if (m_IsRunning)
            return;

        m_IsRunning = true;

        // �⺻������ �ϵ���� ������ ���� ���� ���
        if (threadCount == 0)
        {
            // �ϵ���� �ھ� ���� ����Ͽ� ������ �� ����
            threadCount = std::thread::hardware_concurrency() / 2;
            if (threadCount < 1)
                threadCount = 1;
        }
        // ��Ŀ ������ ����
        for (int32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this]() { WorkerThread(); });
        }

        LOGI << "PacketQueue initialized with " << threadCount << " worker threads";
    }

    // ���� ó��
    void Shutdown()
    {
        if (!m_IsRunning)
            return;

        m_IsRunning = false;

        // �� �۾��� ��� ������ ����ŭ �־� ��� ���� ��������� ����ǵ��� ��
        for (size_t i = 0; i < m_WorkerThreads.size(); ++i)
        {
            PacketTask emptyTask;
            m_TaskQueue.push(emptyTask);
        }

        // ��� ������ ���� ���
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }
        m_WorkerThreads.clear();

        // ť ����
        PacketTask dummyTask;
        while (m_TaskQueue.try_pop(dummyTask)) {}

        LOGI << "PacketQueue shutdown complete";
    }

    // ��Ŷ �۾� �߰�
    void PushPacket(AsioSessionPtr session, const BYTE* buffer, size_t length)
    {
        if (!m_IsRunning || !session)
            return;

        // �� �۾� ����
        PacketTask task;
        task.session = session;
        task.buffer.resize(length);
        std::memcpy(task.buffer.data(), buffer, length);
        task.length = length;

        // �� ���� concurrent_queue�� �۾� �߰�
        m_TaskQueue.push(std::move(task));
    }

    // ���� ť�� �ִ� �۾� �� ��ȯ (�ٻ簪)
    size_t GetQueueSize() const
    {
        return m_TaskQueue.unsafe_size();
    }

private:
    PacketQueue() : m_IsRunning(false) {}
    ~PacketQueue() { Shutdown(); }

    // ��Ŀ ������ �Լ�
    void WorkerThread()
    {
        // �����帶�� �� ���� �Ҵ��ϴ� ��ü
        PacketTask task;

        while (m_IsRunning)
        {
            // concurrent_queue���� �۾� ��������
            if (m_TaskQueue.try_pop(task))
            {
                // ���� ���� Ȯ�� (�� ������ ���� ��ȣ�� ���)
                if (!task.session && !m_IsRunning)
                    break;

                try
                {
                    // ������ ���� ��ȿ���� Ȯ��
                    // ��Ŷ ó��
                    task.session->ProcessPacket(task.buffer.data(), static_cast<int32>(task.length));
                }
                catch (const std::exception& e)
                {
                    LOGE << "Exception in packet worker thread: " << e.what();
                }
                catch (...)
                {
                    LOGE << "Unknown exception in packet worker thread";
                }
            }
            else
            {
                // ť�� ����� �� CPU ���� ���Ҹ� ���� ª�� ���
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic<bool> m_IsRunning;
    Concurrency::concurrent_queue<PacketTask> m_TaskQueue; // lock-free ����
    std::vector<std::thread> m_WorkerThreads;
};