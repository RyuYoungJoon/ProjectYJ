#pragma once
#include <string>

// WinApi로 만든 UI 관련 함수

void InitChatWindow(HINSTANCE hInstance, int nCmdShow);
void RunChatWindow();/*
void UpdateUI(bool isConnected);
void OnClientConnect();
void OnClientDisconnect();
void OnMessageRecv(const std::string& sender, const std::string& message);
void SendChatMessage();
void AddChatMessage(const std::wstring& message);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);*/