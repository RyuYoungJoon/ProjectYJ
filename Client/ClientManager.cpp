﻿#include "pch.h"
#include "ClientSession.h"
#include "ClientManager.h"

extern ClientServicePtr clientService;

ClientManager::ClientManager()
	//: m_Timer(std::make_shared<boost::asio::steady_timer>(clientService->iocontext))
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution<int> dist(10, 100);
	targetRandomCnt = dist(dre);
}

void ClientManager::Init(int32 sessionUid, AsioSessionPtr session)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	if(m_Sessions.find(sessionUid) == m_Sessions.end())
		m_Sessions.insert(std::make_pair(sessionUid, session));
	else
		LOGE << "Already be Session";
	
	m_Service = clientService;

	m_RunningState = RunningState::Connect;
	int32 sessionSize = m_Sessions.size();
	LOGD << "Session Size : " << sessionSize;
	if (sessionSize == 100)
	{
		run = true;
		m_cv.notify_all();
	}
}

void ClientManager::Process()
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_cv.wait(lock, [this]() { return run; });
	lock.unlock();

	while (run)
	{
		switch (m_RunningState)
		{
		case RunningState::Connect:
		case RunningState::Send:
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			for (int i = 0; i < targetRandomCnt; ++i)
			{
				string message("asdvwevljnsdvldsvnklewvlkn", 128);
				Packet packet;
				std::memset(packet.header.checkSum, 0x12, sizeof(packet.header.checkSum));
				std::memset(packet.header.checkSum + 1, 0x34, sizeof(packet.header.checkSum) - 1);
				packet.header.type = PacketType::YJ;
				packet.header.size = static_cast<uint32>(sizeof(PacketHeader) + sizeof(packet.payload) + sizeof(PacketTail));
				std::memcpy(packet.payload, message.c_str(), message.size());
				packet.tail.value = 255;

				clientService->BroadCast(packet);
			}

			m_RunningState = RunningState::Disconnect;
		}
		break;
		case RunningState::Disconnect:
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			// 클라이언트 Disconnect
			StopClient();
			
			m_RunningState = RunningState::Reconnect;
		}
		break;
		case RunningState::Reconnect:
		{
			//run = false;
			std::lock_guard<std::mutex> lock(m_Mutex);

			m_Service->Start();

			m_RunningState = RunningState::Connect;
		}
		break;
		case RunningState::Recv:
			break;
		default:
			break;
		}

		std::this_thread::sleep_for(1s);
	}
}

void ClientManager::StopClient()
{
	// timer cancle
	if (m_Timer)
	{
		boost::system::error_code ec;
		m_Timer->cancel(ec);
		if (ec)
		{
			LOGE << "m_Timer 에러 : " << ec.value() << ", " << ec.message();
		}
	}

	for (auto session : m_Sessions)
	{
		session.second->Disconnect();
		session.second.reset();
	}
	m_Sessions.clear();
}
