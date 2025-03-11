#pragma once

using PacketHandlerFunc = std::function<void(AsioSessionPtr&, const Packet&)>;

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
	void Dispatch(AsioSessionPtr session, const Packet& packet);
	void RegisterHandler(PacketType type, PacketHandlerFunc handler);

private:
    PacketRouter()
        : m_IsRunning(false)
    {
    }
    ~PacketRouter() { Shutdown(); };

    int32 GetWorkerIndex(int32 sessionUID) const;

    std::atomic<bool> m_IsRunning;
    int32 m_NumWorkers;
    std::vector<std::thread> m_WorkerThreads;
    std::vector<std::unique_ptr<boost::lockfree::queue<std::tuple<int32, Packet>>>> m_WorkerQueues;
    std::unordered_map<PacketType, PacketHandlerFunc> m_Handlers;
    std::mutex m_HandlerMutex;
};

class WorkerThread
{
public:
    WorkerThread(int32 id, boost::lockfree::queue<std::tuple<int32, Packet>>* queue,
        std::atomic<bool>* isRunning,
        std::unordered_map<PacketType, PacketHandlerFunc>* handlers);

    ~WorkerThread() = default;

    void Run();
    void ProcessPacket(AsioSessionPtr session, const Packet& packet);

private:
    int32 m_Id;
    boost::lockfree::queue<std::tuple<int32, Packet>>* m_Queue;
    std::atomic<bool>* m_IsRunning;
    std::unordered_map<PacketType, PacketHandlerFunc>* m_Handlers;
};