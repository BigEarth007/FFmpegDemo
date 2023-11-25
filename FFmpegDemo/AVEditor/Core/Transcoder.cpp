#include "pch.h"
#include "Transcoder.h"


namespace aveditor
{
	CTranscoder::CTranscoder(FCache& n_Cache, const int& n_nPrefix, 
		const EStreamType& n_eStreamType)
		: CDecoder(n_Cache, n_nPrefix, n_eStreamType)
	{
	}

	CTranscoder::~CTranscoder()
	{
		Release();
	}

	void CTranscoder::Init(FCodecContext& n_InputCodecContext, 
		FCodecContext& n_OutputCodecContext)
	{
		Release();

		CDecoder::Init(n_InputCodecContext, n_OutputCodecContext);

		// Maybe there are more than one input context, so copy output codec context
		m_OutputCodecContext = new FCodecContext;
		m_OutputCodecContext->CopyCodecParameter(
			n_OutputCodecContext.m_Context);
		m_OutputCodecContext->Open();
	}

	void CTranscoder::Run()
	{
		ThrowExceptionExpr(!m_OutputCodecContext || !m_InputCodecContext,
			"You should call function Init() first.\n");
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		int			ret = 0;
		int			nKeyPrevious = m_nPreviousPrefix + (int)m_eStreamType;
		int			nKeyCurrent = m_nCurrentPrefix + (int)m_eStreamType;

		CQueueItem* QueueItem = m_Cache->GetBufferQueue(nKeyPrevious);
		m_OutputQueue = m_Cache->GetBufferQueue(nKeyCurrent);

		while (!IsStop() && QueueItem)
		{
			ret = DecodePacket(QueueItem, nKeyCurrent);
			if (ret == AVERROR_EOF) break;

			StageSleep();
		}

		m_Convert.CleanAudioFifo(nKeyCurrent);
		FinishedConvert(nullptr, nKeyCurrent);
		m_Cache->Push(nKeyCurrent, (AVPacket*)nullptr);

		Thread::Run();
	}

	void CTranscoder::Release()
	{
		if (m_OutputCodecContext)
			delete m_OutputCodecContext;

		CDecoder::Release();
	}

	int CTranscoder::FinishedConvert(AVFrame* n_Frame, const int& n_nKey)
	{
		int ret = m_OutputCodecContext->EncodeFrame(n_Frame,
			[this, &n_nKey](AVPacket* n_Packet) {

				//LogInfo("StreamIndex: %d; Pts: %lld, Dts: %lld, Duration: %lld\n",
				//	n_Packet->stream_index, n_Packet->pts, n_Packet->dts, n_Packet->duration);

				if (n_Packet->pts < n_Packet->dts)
					n_Packet->pts = n_Packet->dts;

				ConsumeCache(n_nKey);
				m_OutputQueue->Push(n_Packet);

				return 0;
			});

		av_frame_free(&n_Frame);

		return 0;
	}

}
