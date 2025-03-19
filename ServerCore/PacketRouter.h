#pragma once

struct PacketQueueItem {
    int32 sessionId;
    Packet* packet;

    PacketQueueItem() = default;
    PacketQueueItem(int32 id, Packet* p) : sessionId(id), packet(p) {}
};

// PacketRouter에서 큐 벡터 선언 변경

using PacketHandlerFunc = std::function<void(AsioSessionPtr&, Packet*)>;

class PacketProcessor : public enable_shared_from_this<PacketProcessor>
{
public:
    PacketProcessor();

    PacketProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool isRunning,
        std::unordered_map<PacketType, PacketHandlerFunc>* handlers);

    virtual ~PacketProcessor();

    //void SetThreadID(atomic<int32> id)
    void SetProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool isRunning,
        std::unordered_map<PacketType, PacketHandlerFunc>* handlers);
    void Run();
    virtual void HandlePacket(AsioSessionPtr session, const Packet* packet);

    virtual void Test();

private:
    int32 m_Id;
    Concurrency::concurrent_queue<PacketQueueItem>* m_Queue;
    bool m_IsRunning;
    std::unordered_map<PacketType, PacketHandlerFunc>* m_Handlers;
};

using PacketHandlerFuncTest = std::function<shared_ptr<PacketProcessor>()>;

class PacketRouter
{
public:
	static PacketRouter& GetInstance()
	{
		static PacketRouter instance;
		return instance;
	}

	void Init(int32 numThread, PacketHandlerFuncTest initfunc);
	void Shutdown();
	void Dispatch(AsioSessionPtr session, BYTE* buffer);
	void RegisterHandler(PacketType type, PacketHandlerFunc handler);

    shared_ptr<PacketProcessor> CreatePacketHandler(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool isRunning,
        std::unordered_map<PacketType, PacketHandlerFunc>* handlers);

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
    PacketProcessor* m_PacketProcessor = nullptr;

    PacketHandlerFuncTest m_CreateFunc = nullptr;
};
