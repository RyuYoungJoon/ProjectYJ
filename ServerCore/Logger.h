#pragma once
class Logger
{
public:

	static std::mutex m_Mutex;
	static string MyLog(const std::string& strMsg);
};