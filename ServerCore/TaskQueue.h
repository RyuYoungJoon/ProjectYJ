#pragma once
#include <algorithm>
#include "AsioSession.h"

// 작업 큐에 저장될 Task의 우선순위 정의
// 패킷 처리를 위한 작업 단위
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

    // 워커 스레드 풀 초기화
    void Initialize(int32 threadCount = 0)
    {
        if (m_IsRunning)
            return;

        m_IsRunning = true;

        // 기본값으로 하드웨어 스레드 수의 절반 사용
        if (threadCount == 0)
        {
            // 하드웨어 코어 수에 기반하여 스레드 수 결정
            threadCount = std::thread::hardware_concurrency() / 2;
            if (threadCount < 1)
                threadCount = 1;
        }
        // 워커 스레드 생성
        for (int32 i = 0; i < threadCount; ++i)
        {
            m_WorkerThreads.emplace_back([this]() { WorkerThread(); });
        }

        LOGI << "PacketQueue initialized with " << threadCount << " worker threads";
    }

    // 종료 처리
    void Shutdown()
    {
        if (!m_IsRunning)
            return;

        m_IsRunning = false;

        // 빈 작업을 모든 스레드 수만큼 넣어 대기 중인 스레드들이 종료되도록 함
        for (size_t i = 0; i < m_WorkerThreads.size(); ++i)
        {
            PacketTask emptyTask;
            m_TaskQueue.push(emptyTask);
        }

        // 모든 스레드 종료 대기
        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
                thread.join();
        }
        m_WorkerThreads.clear();

        // 큐 비우기
        PacketTask dummyTask;
        while (m_TaskQueue.try_pop(dummyTask)) {}

        LOGI << "PacketQueue shutdown complete";
    }

    // 패킷 작업 추가
    void PushPacket(AsioSessionPtr session, const BYTE* buffer, size_t length)
    {
        if (!m_IsRunning || !session)
            return;

        // 새 작업 생성
        PacketTask task;
        task.session = session;
        task.buffer.resize(length);
        std::memcpy(task.buffer.data(), buffer, length);
        task.length = length;

        // 락 없이 concurrent_queue에 작업 추가
        m_TaskQueue.push(std::move(task));
    }

    // 현재 큐에 있는 작업 수 반환 (근사값)
    size_t GetQueueSize() const
    {
        return m_TaskQueue.unsafe_size();
    }

private:
    PacketQueue() : m_IsRunning(false) {}
    ~PacketQueue() { Shutdown(); }

    // 워커 스레드 함수
    void WorkerThread()
    {
        // 스레드마다 한 번만 할당하는 객체
        PacketTask task;

        while (m_IsRunning)
        {
            // concurrent_queue에서 작업 가져오기
            if (m_TaskQueue.try_pop(task))
            {
                // 종료 조건 확인 (빈 세션은 종료 신호로 사용)
                if (!task.session && !m_IsRunning)
                    break;

                try
                {
                    // 세션이 아직 유효한지 확인
                    // 패킷 처리
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
                // 큐가 비었을 때 CPU 부하 감소를 위한 짧은 대기
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic<bool> m_IsRunning;
    Concurrency::concurrent_queue<PacketTask> m_TaskQueue; // lock-free 구현
    std::vector<std::thread> m_WorkerThreads;
};