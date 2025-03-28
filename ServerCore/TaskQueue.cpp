#include "pch.h"
#include "TaskQueue.h"
#include "SessionManager.h"
#include "AsioSession.h"

void TaskQueue::ProcessIoTask(const IoTask& task)
{
    // SessionManager에서 세션 찾기
    AsioSessionPtr session = SessionManager::GetInstance().GetSession(task.sessionUID);
    if (!session)
    {
        LOGE << "Session not found for UID: " << task.sessionUID;
        return;
    }

    // 상태별 처리
    switch (task.state)
    {
    case NetState::Disconnect:
        LOGI << "Handling disconnect for session: " << task.sessionUID;
        // 실제 연결 종료 처리
        session->CloseSession("IoTaskQueue_Disconnect");
        break;
    case NetState::Reconnect:
        LOGI << "Handling reconnect for session: " << task.sessionUID;
        // 재연결 로직
        // session->Reconnect();
        break;
    default:
        LOGI << "Setting session state to: " << static_cast<int>(task.state);
        session->SetNetState(task.state);
        break;
    }
}
