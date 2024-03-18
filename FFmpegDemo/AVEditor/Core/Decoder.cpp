#include "pch.h"
#include "Decoder.h"


namespace aveditor
{
	CDecoder::CDecoder(FCache& n_Cache, const int& n_nPrefix,
		const EStreamType& n_eStreamType)
		: IStage(n_Cache, n_nPrefix, n_eStreamType)
	{
	}

	CDecoder::~CDecoder()
	{
		Release();
	}

	void CDecoder::Init(FCodecContext& n_InputCodecContext,
		FCodecContext& n_OutputCodecContext)
	{
		Release();

		m_InputCodecContext = &n_InputCodecContext;
		m_OutputCodecContext = &n_OutputCodecContext;

		m_Convert.Init(n_InputCodecContext, n_OutputCodecContext);

		m_Convert.SetFinishedCallback(std::bind(&CDecoder::FinishedConvert, this,
			std::placeholders::_1, std::placeholders::_2));
	}

	void CDecoder::Run()
	{
		ThrowExceptionExpr(!m_InputCodecContext, 
			"You should call function Init() first.\n");
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		int			ret = 0;
		int			nKeyPrevious = m_nPreviousPrefix + (int)m_eStreamType;
		int			nKeyCurrent = m_nCurrentPrefix + (int)m_eStreamType;

		CQueueItem* QueueItem = m_Cache->GetBufferQueue(nKeyPrevious);
		m_OutputQueue = m_Cache->GetBufferQueue(nKeyCurrent);

		while (!IsStop() && QueueItem)
		{
			ret = DoWithQueue(QueueItem, nKeyCurrent);
			if (ret == AVERROR_EOF) break;

			StageSleep();
		}

		m_Convert.CleanAudioFifo(nKeyCurrent);
		m_Cache->Push(nKeyCurrent, (AVFrame*)nullptr);
		
		Thread::Run();
	}

	void CDecoder::Release()
	{
		m_InputCodecContext = nullptr;
		m_OutputCodecContext = nullptr;

		m_Convert.Release();
	}

	int CDecoder::DoWithQueue(CQueueItem* n_QueueItem, const int n_nCurrentKey)
	{
		int ret = 0;

		int nIndex = GetContextIndex();
		auto ContextInfo = m_Cache->GetContextInfo(nIndex);

		if (!ContextInfo || !ContextInfo->bContextEmpty)
			ret = DecodePacket(n_QueueItem, n_nCurrentKey);
		else
			ret = CoverFrame(n_QueueItem, n_nCurrentKey);

		return ret;
	}

	int CDecoder::CoverFrame(CQueueItem* n_QueueItem, const int n_nCurrentKey)
	{
		AVFrame* Frame = nullptr;

		int ret = m_Cache->Pop(n_QueueItem, Frame);
		if (ret < 0)
		{
			Sleep(kSleepDelay);
			return ret;
		}

		if (Frame)
		{
			int ret2 = m_Convert.Process(Frame, n_nCurrentKey);
			if (0 != ret2) av_frame_free(&Frame);
		}
		else
		{
			ret = AVERROR_EOF;
		}

		return ret;
	}

	int CDecoder::DecodePacket(CQueueItem* n_QueueItem, const int n_nCurrentKey)
	{
		AVPacket* Packet = nullptr;

		int ret = m_Cache->Pop(n_QueueItem, Packet);
		if (ret < 0)
		{
			Sleep(kSleepDelay);
			return ret;
		}

		//if (Packet)
		//{
		//	LogInfo("Index: %d, Pts: %lld, Dts: %lld, Duration: %lld\n", 
		//		Packet->stream_index, Packet->pts, Packet->dts, Packet->duration);
		//}

		ret = m_InputCodecContext->DecodePacket(Packet,
			[this, &n_nCurrentKey](AVFrame* n_Frame) {

				if (m_OutputCodecContext &&
					n_Frame->pict_type == AVPictureType::AV_PICTURE_TYPE_B &&
					m_InputCodecContext->m_Context->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO &&
					(m_OutputCodecContext->m_Context->codec->capabilities & AV_CODEC_CAP_DELAY) != 0)
				{
					n_Frame->pict_type = AVPictureType::AV_PICTURE_TYPE_P;
				}

				if (n_Frame->pts <= 0 &&
					m_InputCodecContext->m_Context->codec_id == AVCodecID::AV_CODEC_ID_MP3 &&
					m_InputCodecContext->m_Context->frame_size > n_Frame->nb_samples)
				{
					return 1;
				}

				//LogInfo("Decode frame: %d, Pts: %lld.\n", n_nCurrentKey, n_Frame->pts);
				n_Frame->pts = n_Frame->best_effort_timestamp;
				int ret = m_Convert.Process(n_Frame, n_nCurrentKey);

				return ret;
			});

		av_packet_free(&Packet);

		return ret;
	}

	int CDecoder::FinishedConvert(AVFrame* n_Frame, const int& n_nKey)
	{
		ConsumeCache(n_nKey);

		if (m_OutputQueue) m_OutputQueue->Push(n_Frame);
		else m_Cache->Push(n_nKey, n_Frame);

		return 0;
	}

}