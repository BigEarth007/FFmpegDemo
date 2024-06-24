#include "pch.h"
#include <functional>
#include "Thread.h"

namespace aveditor
{
	Thread::Thread()
	{
	}

	Thread::~Thread()
	{
		Join();
	}

	void Thread::Start()
	{
		Join();
		if (m_bStop) {
			m_bStop = false;
			m_pThread = std::thread(std::bind(&Thread::Run, this));
		}
	}

	void Thread::Stop()
	{
		m_bStop = true;
	}

	void Thread::Join()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_pThread.joinable())
			m_pThread.join();
	}

	void Thread::Run()
	{
		Stop();

		if (m_funcFinished) m_funcFinished();
	}

	void Thread::PreRun()
	{
		if (m_funcStartup) m_funcStartup();
	}

	bool Thread::IsStop()
	{
		bool b = m_bStop;
		return b;
	}

	void Thread::Sleep(uint32_t n_nDuration, ESleep n_eSleep /*= ESleep::ES_Millisecond*/)
	{
		switch (n_eSleep)
		{
		case ESleep::ES_Second:
			std::this_thread::sleep_for(std::chrono::seconds(n_nDuration));
			break;
		case ESleep::ES_Millisecond:
			std::this_thread::sleep_for(std::chrono::milliseconds(n_nDuration));
			break;
		case ESleep::ES_Microsecond:
			std::this_thread::sleep_for(std::chrono::microseconds(n_nDuration));
			break;
		case ESleep::ES_Nanosecond:
			std::this_thread::sleep_for(std::chrono::nanoseconds(n_nDuration));
			break;
		}
	}

}
