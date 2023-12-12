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
		if (m_bStop) return true;
		
		return false;
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
