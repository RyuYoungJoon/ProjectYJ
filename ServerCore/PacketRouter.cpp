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

    // ��Ŀ �� ���� (�⺻��: CPU �ھ� �� / 2)
    if (numThread <= 0)
    {
        numThread = std::thread::hardware_concurrency() / 2;
        if (numThread < 1) numThread = 1;
    }

    m_NumWorkers = numThread;

    // ��Ŀ�� ť ����
    for (int32 i = 0; i < m_NumWorkers; ++i)
    {
        m_PacketQueue.push_back(std::make_unique<Concurrency::concurrent_queue<PacketQueueItem>>());
    }

    // ��Ŀ ������ ����
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

    // ��Ŀ ������ ���� ���
    for (auto& thread : m_WorkerThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    // ť ����
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

    // ť�� ��Ŷ �׸� �߰�
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
        // ť���� ��Ŷ ��������
        if (!m_ProcessQueue->try_pop(item))
        {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        int32 sessionId = item.sessionId;

        // ���� ã��
        AsioSessionPtr session = SessionManager::GetInstance().GetSession(sessionId);
        if (session) {
            HandlePacket(session, item.buffer, item.len);
            //LOGI << "Packet Queue Size : " << m_Queue->unsafe_size();

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