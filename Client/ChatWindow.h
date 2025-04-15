#pragma once
#include <string>

// WinApi�� ���� UI ���� �Լ�

void InitChatWindow(HINSTANCE hInstance, int nCmdShow);
void RunChatWindow();
void UpdateUI();
void OnClientConnect(bool isConnect);
void OnClientDisconnect();
void OnMessageRecv(const std::string& sender, const std::string& message);
void SendChatMessage();
void AddChatMessage(const std::wstring& message);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);