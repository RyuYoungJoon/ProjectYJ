#pragma once

struct PacketQueueItem {
    int32 sessionId;
    Packet* packet;

    PacketQueueItem() = default;
    PacketQueueItem(int32 id, Packet* p) : sessionId(id), packet(p) {}
};

// PacketRouter에서 큐 벡터 선언 변경


class PacketProcessor : public enable_shared_from_this<PacketProcessor>
{
public:
    PacketProcessor();
    virtual ~PacketProcessor();

    void SetProcessor(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool& isRunning);
    void Run();
    virtual void HandlePacket(AsioSessionPtr session, Packet* packet);

private:
    int32 m_Id;
    Concurrency::concurrent_queue<PacketQueueItem>* m_ProcessQueue;
    bool* m_IsRunning;
};

using PacketHandlerFunc = std::function<shared_ptr<PacketProcessor>()>;

class PacketRouter
{
public:
	static PacketRouter& GetInstance()
	{
		static PacketRouter instance;
		return instance;
	}

	void Init(int32 numThread, PacketHandlerFunc initfunc);
	void Shutdown();
	void Dispatch(AsioSessionPtr session, BYTE* buffer);

    shared_ptr<PacketProcessor> CreatePacketHandler(int32 id, Concurrency::concurrent_queue<PacketQueueItem>* queue,
        bool isRunning);

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
    std::vector<std::unique_ptr<Concurrency::concurrent_queue<PacketQueueItem>>> m_PacketQueue;
    std::mutex m_HandlerMutex;
    PacketHandlerFunc m_CreateFunc = nullptr;
};
