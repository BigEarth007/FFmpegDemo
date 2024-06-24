#pragma once
#include <thread>
#include <atomic>
#include <mutex>


namespace aveditor 
{
	enum class ESleep
	{
		// Second
		ES_Second = 0,
		// Milli second
		ES_Millisecond,
		// Micro second
		ES_Microsecond,
		// Nanosecond
		ES_Nanosecond,
	};

	class AVEDITOR_API Thread
	{
	public:
		Thread();
		virtual ~Thread();

		//base option
		virtual void Start();

		virtual void Stop();

		virtual void Join();

		//TO DO
		virtual void Run();
		void PreRun();

		virtual bool IsStop();

		void Sleep(uint32_t n_nDuration, ESleep n_eSleep = ESleep::ES_Millisecond);

		void SetStartupCallback(std::function<void()> n_func) { m_funcStartup = n_func; }
		void SetFinishedCallback(std::function<void()> n_func) { m_funcFinished = n_func; }

	protected:
		Thread(const Thread&) = delete;
		Thread& operator=(const Thread&) = delete;

	protected:
		std::atomic_bool	m_bStop = true;
		std::thread		m_pThread;
		std::mutex		m_mutex;

		std::function<void()> m_funcStartup = nullptr;
		std::function<void()> m_funcFinished = nullptr;
	};

}
