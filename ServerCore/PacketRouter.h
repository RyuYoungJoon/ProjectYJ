#pragma once

struct PacketQueueItem {
    int32 sessionId;
    Packet* packet;

    PacketQueueItem() = default;
    PacketQueueItem(int32 id, Packet* p) : sessionId(id), packet(p) {}
};

// PacketRouter에서 큐 벡터 선언 변경

using PacketHandlerFunc = std::function<void(AsioSessionPtr&, Packet*)>;

class PacketRouter
{
public:
	static PacketRouter& GetInstance()
	{
		static PacketRouter instance;
		return instance;
	}

	void Init(int32 numThread = 0);
	void Shutdown();
	void Dispatch(AsioSessionPtr session, BYTE* buffer);
	void RegisterHandler(PacketType type, PacketHandlerFunc handler);

private:
    PacketRouter()
        : m_IsRunning(false)
    {
        m_NumWorkers = 0;
    }
    ~PacketRouter() { Shutdown(); };

    int32 GetWorkerIndex(int32 sessionUID) const;

    bool m_IsRunning = false;
    int32 m_NumWorkers;
    std::vector<std::thread> m_WorkerThreads;
    std::vector<std::unique_ptr<Concurrency::concurrent_queue<PacketQueueItem>>> m_WorkerQueues;
    std::unordered_map<PacketType, PacketHandlerFunc> m_Handlers;
    std::mutex m_HandlerMutex;
};

class WorkerThread
{
public:
    WorkerThread(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool isRunning,
        std::unordered_map<PacketType, PacketHandlerFunc>* handlers);

    ~WorkerThread() = default;

    void Run();
    void ProcessPacket(AsioSessionPtr session, Packet* packet);

private:
    int32 m_Id;
    Concurrency::concurrent_queue<PacketQueueItem>* m_Queue;
    bool m_IsRunning;
    std::unordered_map<PacketType, PacketHandlerFunc>* m_Handlers;
};