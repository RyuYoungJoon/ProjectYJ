#include "pch.h"
#include "AsioIoContext.h"


AsioIoContext::AsioIoContext(boost::asio::io_context& ioContext)
	: m_ioContext(ioContext)
{
    m_work = nullptr;
}


AsioIoContext::~AsioIoContext()
{
    if (m_work)
        delete m_work;
}

bool AsioIoContext::Init()
{
    m_work = new boost::asio::executor_work_guard<boost::asio::io_context::executor_type>(m_ioContext.get_executor());

    return true;
}

bool AsioIoContext::Run()
{
    uint8 iCnt = std::thread::hardware_concurrency() / 2;

    for (int i = 0; i < iCnt; ++i)
    {
        std::thread* tThread = new std::thread([this] {
            m_ioContext.run();
            });
        m_asioThread.push_back(tThread);
    }

    return true;
}

bool AsioIoContext::Stop()
{
    delete m_work;
    m_work = nullptr;

    return true;
}
