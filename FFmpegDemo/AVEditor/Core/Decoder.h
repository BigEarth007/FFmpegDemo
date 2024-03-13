#pragma once


namespace aveditor
{
	class AVEDITOR_API CDecoder : public IStage
	{
	public:
		CDecoder(FCache& n_Cache, const int& n_nPrefix,
			const EStreamType& n_eStreamType);
		~CDecoder();

		virtual void Init(FCodecContext& n_InputCodecContext, 
			FCodecContext& n_OutputCodecContext);

		virtual void Run();

		virtual void Release();

	protected:
		int DoWithQueue(CQueueItem* n_QueueItem, const int n_nCurrentKey);
		// If the input file is empty, and user writes PCM data into input file
		// Then in the DEMUXER stage, it is AVFrame in the cache instead of AVPacket
		int CoverFrame(CQueueItem* n_QueueItem, const int n_nCurrentKey);
		// Decode AVPacket that read from input context
		int DecodePacket(CQueueItem* n_QueueItem, const int n_nCurrentKey);
		// Decode AVPacket into AVFrame, then send to AVFrame buffer queue
		virtual int FinishedConvert(AVFrame* n_Frame, const int& n_nKey);

	protected:
		// Decode codec context
		FCodecContext*	m_InputCodecContext = nullptr;
		// Encode codec context
		FCodecContext*	m_OutputCodecContext = nullptr;
		// Frame converter
		CConvert		m_Convert;
		// The buffer queue for decode frame
		CQueueItem*		m_OutputQueue = nullptr;
	};
}
