#include "pch.h"
#include "PacketRouter.h"
#include "AsioSession.h"
#include "SessionManager.h"

void PacketRouter::Init(int32 numThread)
{
    if (m_IsRunning)
        return;

    m_IsRunning = true;

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
        // boost::lockfree::queue ���� (ũ�� 1024)
        m_WorkerQueues.push_back(std::make_unique<boost::lockfree::queue<std::tuple<int32, Packet>>>(1024));
    }

    // ��Ŀ ������ ����
    for (int32 i = 0; i < m_NumWorkers; ++i)
    {
        m_WorkerThreads.emplace_back([this, i]() {
            WorkerThread worker(i, m_WorkerQueues[i].get(), &m_IsRunning, &m_Handlers);
            worker.Run();
            });
    }

    LOGI << "PacketDispatcher initialized with " << m_NumWorkers << " worker threads";
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
    m_WorkerQueues.clear();
    m_WorkerThreads.clear();

    LOGI << "PacketDispatcher shutdown complete";
}

void PacketRouter::Dispatch(AsioSessionPtr session, const Packet& packet)
{
    if (!session || !m_IsRunning)
        return;

    int32 sessionId = session->GetSessionUID();
    int32 workerIdx = GetWorkerIndex(sessionId);

    // boost::lockfree::queue�� ���縦 �ϹǷ� ��Ŷ�� �����ؾ� ��
    std::tuple<int32, Packet> item(sessionId, packet);

    // ť�� �߰� �õ�
    if (!m_WorkerQueues[workerIdx]->push(item))
    {
        // ť�� ���� �� ���
        LOGE << "Worker queue " << workerIdx << " is full, packet dropped for session " << sessionId;
    }
}

void PacketRouter::RegisterHandler(PacketType type, PacketHandlerFunc handler)
{
    std::lock_guard<std::mutex> lock(m_HandlerMutex);
    m_Handlers[type] = handler;
}

int32 PacketRouter::GetWorkerIndex(int32 sessionUID) const
{
    return sessionUID % m_NumWorkers;
}

WorkerThread::WorkerThread(int32 id, boost::lockfree::queue<std::tuple<int32, Packet>>* queue, std::atomic<bool>* isRunning, std::unordered_map<PacketType, PacketHandlerFunc>* handlers)
{
    
}

void WorkerThread::Run()
{
    LOGI << "Worker thread " << m_Id << " started";

    std::tuple<int32, Packet> item;

    while (*m_IsRunning)
    {
        // ť���� ��Ŷ ��������
        bool hasWork = false;

        // ��ġ ó�� (�ִ� 32�� ��Ŷ �ѹ��� ó��)
        for (int i = 0; i < 32 && m_Queue->pop(item); ++i)
        {
            hasWork = true;
            int32 sessionId = std::get<0>(item);
            const Packet& packet = std::get<1>(item);

            // ���� ã��
            AsioSessionPtr session = SessionManager::GetInstance().GetSession(sessionId);
            if (session)
            {
                ProcessPacket(session, packet);
            }
        }

        // �۾��� ������ CPU �纸
        if (!hasWork)
        {
            std::this_thread::yield();
        }
    }

    LOGI << "Worker thread " << m_Id << " stopped";
}

void WorkerThread::ProcessPacket(AsioSessionPtr session, const Packet& packet)
{
    PacketType type = packet.header.type;

    // �ڵ鷯 ã��
    auto it = m_Handlers->find(type);
    if (it != m_Handlers->end())
    {
        try
        {
            // ��Ŷ ó�� �ڵ鷯 ȣ��
            it->second(session, packet);
        }
        catch (const std::exception& e)
        {
            LOGE << "Exception in packet handler: " << e.what();
        }
        catch (...)
        {
            LOGE << "Unknown exception in packet handler";
        }
    }
    else
    {
        LOGE << "No handler registered for packet type " << static_cast<int>(type);
    }
}
