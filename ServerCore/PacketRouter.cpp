#include "pch.h"
#include "PacketRouter.h"
#include "AsioSession.h"
#include "SessionManager.h"
#include "ServerAnalyzer.h"

void PacketRouter::Init(int32 numThread, PacketHandlerFunc initfunc)
{
    if (m_IsRunning)
        return;

    m_IsRunning = true;

    m_CreateFunc = initfunc;

    // 워커 수 설정 (기본값: CPU 코어 수 / 2)
    if (numThread <= 0)
    {
        numThread = std::thread::hardware_concurrency() / 2;
        if (numThread < 1) numThread = 1;
    }

    m_NumWorkers = numThread;

    // 워커별 큐 생성
    for (int32 i = 0; i < m_NumWorkers; ++i)
    {
        m_PacketQueue.push_back(std::make_unique<Concurrency::concurrent_queue<PacketQueueItem>>());
    }

    // 워커 스레드 생성
    for (int32 i = 0; i < m_NumWorkers; ++i)
    {
        m_WorkerThreads.emplace_back([this, i]() {
            auto processor = CreatePacketHandler(i, m_PacketQueue[i].get(), m_IsRunning);
            processor->Run();
            });
    }

    LOGI << "PacketRouter initialized with " << m_NumWorkers << " worker threads";
}

void PacketRouter::Shutdown()
{
    if (!m_IsRunning)
        return;

    m_IsRunning = false;

    // 워커 스레드 종료 대기
    for (auto& thread : m_WorkerThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    // 큐 정리
    m_PacketQueue.clear();
    m_WorkerThreads.clear();

    LOGI << "PacketRouter shutdown complete";
}

void PacketRouter::Dispatch(AsioSessionPtr session, BYTE* buffer, int32 len)
{
    if (!session || !m_IsRunning)
        return;

    int32 sessionId = session->GetSessionUID();
    int32 workerIdx = GetWorkerIndex(sessionId);

    // 큐에 패킷 항목 추가
    PacketQueueItem item(sessionId, buffer, len);
    m_PacketQueue[workerIdx]->push(item);
}

shared_ptr<PacketProcessor> PacketRouter::CreatePacketHandler(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue, bool isRunning)
{
    shared_ptr<PacketProcessor> processor = m_CreateFunc();
    processor->SetProcessor(id, m_PacketQueue[id].get(), m_IsRunning);

    return processor;
}

int32 PacketRouter::GetWorkerIndex(int32 sessionUID) const
{
    return sessionUID % m_NumWorkers;
}

PacketProcessor::PacketProcessor()
{
    LOGD << "PacketProcessor Init";
}

PacketProcessor::~PacketProcessor()
{
}

void PacketProcessor::SetProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue, bool& isRunning)
{
    m_Id = id;
    m_ProcessQueue = queue;
    m_IsRunning = &isRunning;
}

void PacketProcessor::Run()
{
    LOGI << "Worker thread " << m_Id << " started";

    PacketQueueItem item;

    while (*m_IsRunning) {
        // 큐에서 패킷 가져오기
        if (!m_ProcessQueue->try_pop(item))
        {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        int32 sessionId = item.sessionId;

        // 세션 찾기
        AsioSessionPtr session = SessionManager::GetInstance().GetSession(sessionId);
        if (session) {
            HandlePacket(session, item.buffer, item.len);
            //LOGI << "Packet Queue Size : " << m_Queue->unsafe_size();

            // 템플릿화 하면서 PacketPool도 리펙토링 필요.
            //PacketPool::GetInstance().Push(packet);
        }
    }

    LOGE << "Worker thread " << m_Id << " stopped";
}

bool PacketProcessor::HandlePacket(AsioSessionPtr& session, BYTE* packet, int32 len)
{
    LOGD << "HandlePacket";
    return true;
}