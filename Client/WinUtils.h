#pragma once
class WinUtils
{
public:
    // ���ڿ� ��ȯ ��ƿ��Ƽ
    static std::wstring StringToWString(const std::string& str);
    static std::string WStringToString(const std::wstring& wstr);
};

