#include "pch.h"
#include "Encoder.h"


namespace aveditor
{
	CEncoder::CEncoder(FCache& n_Cache, const int& n_nPrefix,
		const EStreamType& n_eStreamType)
		: IStage(n_Cache, n_nPrefix, n_eStreamType)
	{
	}

	CEncoder::~CEncoder()
	{
		Release();
	}

	void CEncoder::Init(AVCodecContext* n_CodecContext)
	{
		// Maybe there are more than one input context, so copy output codec context
		m_OutputCodecContext.CopyCodecParameter(n_CodecContext);
		m_OutputCodecContext.Open();
	}

	void CEncoder::Run()
	{
		ThrowExceptionExpr(!m_OutputCodecContext.m_Context, 
			"You should call function Init() first.\n");
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");
		
		int			ret = 0;
		int			nKeyPrevious = m_nPreviousPrefix + (int)m_eStreamType;
		int			nKeyCurrent = m_nCurrentPrefix + (int)m_eStreamType;
		AVFrame*	Frame = nullptr;

		CQueueItem* QueueItem = m_Cache->GetBufferQueue(nKeyPrevious);
		CQueueItem* OutputQueue = m_Cache->GetBufferQueue(nKeyCurrent);

		while (!IsStop() && QueueItem)
		{
			ret = m_Cache->Pop(QueueItem, Frame);
			if (ret < 0) continue;

			ret = m_OutputCodecContext.EncodeFrame(Frame,  
				[this, &nKeyCurrent, &OutputQueue](AVPacket* n_Packet) {

					//LogInfo("StreamIndex: %d; Pts: %lld, Dts: %lld, Duration: %lld\n",
					//	n_Packet->stream_index, n_Packet->pts, n_Packet->dts, n_Packet->duration);

					if (n_Packet->pts < n_Packet->dts)
						n_Packet->pts = n_Packet->dts;

					ConsumeCache(nKeyCurrent);
					OutputQueue->Push(n_Packet);

					return 0;
				});

			av_frame_free(&Frame);

			if (ret == AVERROR_EOF) break;

			StageSleep();
		}

		// End with nullptr
		m_Cache->Push(nKeyCurrent, (AVPacket*)nullptr);

		Thread::Run();
	}

	void CEncoder::Release()
	{
		m_OutputCodecContext.Release();
	}
}
