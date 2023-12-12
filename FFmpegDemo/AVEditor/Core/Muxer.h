#pragma once


namespace aveditor
{
	class AVEDITOR_API CMuxer : public IStage
	{
	public:
		CMuxer(FCache& n_Cache, const int& n_nPrefix);
		~CMuxer();

		void Init(FFormatContext& n_OutputContext);

		virtual void Run();

	protected:
		struct FPacketCache
		{
			AVPacket*	Packet = nullptr;
			CQueueItem* QueueItem = nullptr;
			double		dTimestamp = 0.0f;
			double		dTimebase = 0.0f;
			// -1: not use
			// 0: using
			// 1: finished
			int			nStatus = -1;
		};

		// Write all AVPacket into output context
		void Muxing(const int n_nIndex, const FContextInfo& n_ContextInfo);

	protected:
		// Output file
		FFormatContext* m_OutputContext = nullptr;
		// The duration of last input context
		double			m_dDuration = 0.0;
	};
}
