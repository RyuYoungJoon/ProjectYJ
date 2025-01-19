#include "pch.h"
#include "MyLogger.h"

std::mutex MyLogger::m_Mutex;

string MyLogger::MyLog(const std::string& strMsg)
{
    std::lock_guard<std::mutex> Lock(m_Mutex);

    std::string		   my_str;
    std::ostringstream os;
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::tm local_time;
    localtime_s(&local_time, &now_time_t);

    os << "[" << local_time.tm_year + 1900 << "-" << std::setfill('0') << std::setw(2) << local_time.tm_mon + 1 << "-" << std::setfill('0') << std::setw(2) << local_time.tm_mday << " ";
    os << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "." << std::setfill('0') << std::setw(3) << milliseconds.count()<< "]" << "#";
    os << "#" << strMsg << std::endl;

    my_str = os.str();
    OutputDebugStringA(my_str.c_str());

    return os.str();
}
