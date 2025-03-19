#include "pch.h"
#include "PacketRouter.h"
#include "AsioSession.h"
#include "SessionManager.h"
#include "ServerAnalyzer.h"
#include "MemoryPoolManager.h"

void PacketRouter::Init(int32 numThread, PacketHandlerFuncTest initfunc)
{
    if (m_IsRunning)
        return;

    m_IsRunning = true;

    m_CreateFunc = initfunc;

    //m_PacketProcessor = processor.get();
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
        m_WorkerQueues.push_back(std::make_unique<Concurrency::concurrent_queue<PacketQueueItem>>());
    }

    // 워커 스레드 생성
    for (int32 i = 0; i < m_NumWorkers; ++i)
    {
        m_WorkerThreads.emplace_back([this, i]() {
            auto processor = CreatePacketHandler(i, m_WorkerQueues[i].get(), m_IsRunning, &m_Handlers);
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
    m_WorkerQueues.clear();
    m_WorkerThreads.clear();

    LOGI << "PacketRouter shutdown complete";
}

void PacketRouter::Dispatch(AsioSessionPtr session, BYTE* buffer)
{
    if (!session || !m_IsRunning)
        return;

    Packet* packet = reinterpret_cast<Packet*>(buffer);

    Packet* packetCopy = static_cast<Packet*>(MemoryPoolManager::GetMemoryPool(packet->header.size).Allocate());
    memcpy(packetCopy, packet, packet->header.size);

    int32 sessionId = session->GetSessionUID();
    int32 workerIdx = GetWorkerIndex(sessionId);

    // 큐에 패킷 항목 추가
    PacketQueueItem item(sessionId, packetCopy);
    m_WorkerQueues[workerIdx]->push(item);
}

void PacketRouter::RegisterHandler(PacketType type, PacketHandlerFunc handler)
{
    std::lock_guard<std::mutex> lock(m_HandlerMutex);

    auto it = m_Handlers.find(type);
    if (it == m_Handlers.end())
        m_Handlers.insert(std::make_pair(type, handler));
}

shared_ptr<PacketProcessor> PacketRouter::CreatePacketHandler(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue, bool isRunning, std::unordered_map<PacketType, PacketHandlerFunc>* handlers)
{
    std::lock_guard<std::mutex> lock(m_HandlerMutex);

    shared_ptr<PacketProcessor> processor = m_CreateFunc();
    processor->SetProcessor(id, m_WorkerQueues[id].get(), m_IsRunning, &m_Handlers);

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

PacketProcessor::PacketProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue, bool isRunning, std::unordered_map<PacketType, PacketHandlerFunc>* handlers)
    :m_Id(id), m_Queue(queue), m_IsRunning(isRunning), m_Handlers(handlers)
{
}

PacketProcessor::~PacketProcessor()
{
}

void PacketProcessor::SetProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue, bool isRunning, std::unordered_map<PacketType, PacketHandlerFunc>* handlers)
{
    m_Id = id;
    m_Queue = queue;
    m_IsRunning = isRunning;
    m_Handlers = handlers;
}

void PacketProcessor::Run()
{
    LOGI << "Worker thread " << m_Id << " started";

    PacketQueueItem item;

    while (m_IsRunning) {
        // 큐에서 패킷 가져오기
        bool hasWork = false;

        if (m_Queue->try_pop(item)) {
            hasWork = true;
            int32 sessionId = item.sessionId;
            Packet* packet = item.packet;

            // 세션 찾기
            AsioSessionPtr session = SessionManager::GetInstance().GetSession(sessionId);
            if (session) {
                HandlePacket(session, packet);
                LOGI << "Packet Queue Size : " << m_Queue->unsafe_size();

                MemoryPoolManager::GetMemoryPool(packet->header.size).Deallocate(packet);
            }
        }

        // 작업이 없으면 잠시 대기
        if (!hasWork) {
            std::this_thread::yield();
        }
    }

    LOGE << "Worker thread " << m_Id << " stopped";
}

void PacketProcessor::HandlePacket(AsioSessionPtr session, const Packet* packet)
{
    LOGD << "HandlePacket";
}

void PacketProcessor::Test()
{
    LOGD << "PacketProcessor";
}
