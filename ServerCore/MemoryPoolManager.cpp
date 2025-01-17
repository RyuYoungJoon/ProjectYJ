#include "pch.h"
#include "MemoryPoolManager.h"

std::map<size_t, std::unique_ptr<MemoryPool>> MemoryPoolManager::m_Pools;
std::mutex MemoryPoolManager::m_Mutex;