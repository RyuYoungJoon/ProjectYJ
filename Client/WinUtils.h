#pragma once
class WinUtils
{
public:
    // 문자열 변환 유틸리티
    static std::wstring StringToWString(const std::string& str);
    static std::string WStringToString(const std::wstring& wstr);
};

