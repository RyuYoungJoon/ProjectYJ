#pragma once
class SessionManager
{
public:
    static SessionManager& GetInstance()
    {
        static SessionManager instance;
        return instance;
    }

    void RegisterSession(AsioSessionPtr session);
    void UnregisterSession(AsioSessionPtr session);
    AsioSessionPtr GetSession(int32 sessionId);

private:
    SessionManager() = default;
    ~SessionManager() = default;

    std::mutex m_Mutex;
    std::unordered_map<int32, AsioSessionPtr> m_Sessions;
};
