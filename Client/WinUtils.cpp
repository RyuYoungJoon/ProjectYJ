#include "pch.h"
#include "WinUtils.h"
#include <CommCtrl.h>

std::wstring WinUtils::StringToWString(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len <= 0) return L"";

    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf.data(), len);
    return std::wstring(buf.data());
}

std::string WinUtils::WStringToString(const std::wstring& wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";

    std::vector<char> buf(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf.data(), len, NULL, NULL);
    return std::string(buf.data());
}