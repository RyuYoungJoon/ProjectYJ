#include "pch.h"
#include "TaskQueue.h"
#include "SessionManager.h"
#include "AsioSession.h"

void TaskQueue::ProcessIoTask(IoTask task)
{
    // SessionManager���� ���� ã��
    AsioSessionPtr session = SessionManager::GetInstance().GetSession(task.sessionUID);
    if (!session)
    {
        LOGE << "Session not found for UID: " << task.sessionUID;
        return;
    }

    // ���º� ó��
    switch (task.state)
    {
    case NetState::Disconnect:
        LOGI << "Handling disconnect for session: " << task.sessionUID;
        // ���� ���� ���� ó��
        session->CloseSession("IoTaskQueue_Disconnect");
        break;
    case NetState::Reconnect:
        LOGI << "Handling reconnect for session: " << task.sessionUID;
        // �翬�� ����
        // session->Reconnect();
        break;
    default:
        LOGI << "Setting session state to: " << static_cast<int>(task.state);
        session->SetNetState(task.state);
        break;
    }
}
