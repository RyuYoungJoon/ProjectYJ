#pragma once
class Logger
{
public:

	static std::mutex m_Mutex;
	static string DLog(const std::string& strMsg);
};