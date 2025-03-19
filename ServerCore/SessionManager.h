#pragma once
class SessionManager
{
public:
    static SessionManager& GetInstance()
    {
        static SessionManager instance;
        return instance;
    }

    void AddSession(AsioSessionPtr session);
    void RemoveSession(AsioSessionPtr session);
    AsioSessionPtr GetSession(int32 sessionId);

private:
    SessionManager();
    ~SessionManager();

    std::mutex m_Mutex;
    std::unordered_map<int32, AsioSessionPtr> m_Sessions;
};
