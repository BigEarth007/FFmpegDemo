#include "pch.h"
#include "Demuxer.h"


namespace aveditor
{
	CDemuxer::CDemuxer(FCache& n_Cache, const int& n_nPrefix)
		: IStage(n_Cache, n_nPrefix, EStreamType::EST_Max)
	{
	}

	CDemuxer::~CDemuxer()
	{
		Release();
	}

	void CDemuxer::Init(FFormatContext& n_InputContext, FFormatContext* n_OutputContext)
	{
		m_InputContext = &n_InputContext;
		if (n_OutputContext->m_Context)
			m_OutputContext = n_OutputContext;
	}

	void CDemuxer::Release()
	{
	}

	void CDemuxer::SelectSection(const double n_dStart, const double n_dDuration)
	{
		if (n_dStart > m_InputContext->Duration())
			return;

		m_dSectionFrom = n_dStart;
		if (n_dDuration > 0)
		{
			m_dSectionTo = n_dStart + n_dDuration;
		}
	}

	void CDemuxer::SetCallbackFillVideoFrame(
		std::function<void(AVFrame*, const void*, const int&)> n_func)
	{
		m_funcFillVideoFrame = n_func;
	}

	void CDemuxer::SetCallbackFillAudioFrame(
		std::function<void(AVFrame*, const void*, const int&)> n_func)
	{
		m_funcFillAudioFrame = n_func;
	}

	void CDemuxer::WriteFrameDatas(EStreamType n_eStreamType,
		const void* n_Data, const int& n_nSize)
	{
		if (!m_InputContext || !m_bIsInputCtxEmpty) return;

		int nKey = m_nCurrentPrefix + (int)n_eStreamType;
		CQueueItem* QueueItem = m_Cache->GetBufferQueue(nKey);
		if (!QueueItem) return;

		if (!n_Data || n_nSize == 0)
		{
			QueueItem->Push((AVFrame*)nullptr);
			return;
		}

		AVCodecContext* CodecContext =
			m_InputContext->GetCodecContext(n_eStreamType);
		if (!CodecContext) return;

		AVFrame* Frame = nullptr;

		if (n_eStreamType == EStreamType::EST_Video)
		{
			Frame = WriteVideoFrame(CodecContext, n_Data, n_nSize);
		}
		else if (n_eStreamType == EStreamType::EST_Audio)
		{
			Frame = WriteAudioFrame(CodecContext, n_Data, n_nSize);
		}

		if (Frame) QueueItem->Push(Frame);
	}

	void CDemuxer::Run()
	{
		ThrowExceptionExpr(!m_InputContext, 
			"You should call function Init() first.\n");
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		auto&		vContextInfo = m_Cache->GetContextInfos();
		int		nContextIndex = GetContextIndex();
		m_bIsInputCtxEmpty = vContextInfo[nContextIndex].bContextEmpty;
		if (m_bIsInputCtxEmpty) return;

		int		ret = 0;
		int		nKey = 0;
		int64_t		nPts = 0;
		double		dSecond = 0;
		AVStream*	Stream = nullptr;
		AVPacket*	Packet = av_packet_alloc();

		EStreamType eStreamType = EStreamType::EST_Max;

		SetDuration();

		std::map<EStreamType, FCodecContext>* mOutputCodecContext = nullptr;
		if (m_OutputContext)
			mOutputCodecContext = m_OutputContext->GetCodecContext();

		CQueueItem* QueueItems[(int)EStreamType::EST_Max] = { nullptr };
		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
			QueueItems[i] = nullptr;

		while (!IsStop())
		{
			int nSelectedContextIndex = m_Cache->GetSelectedContextIndex();
			if (nContextIndex != nSelectedContextIndex &&
				vContextInfo[nContextIndex].eJob != EJob::EJ_AMixBranch)
			{
				Sleep(kSleepDelay * 10);
				continue;
			}

			ret = m_InputContext->ReadPacket(Packet);
			if (ret < 0) break;

			//if (Packet->stream_index == 0)
			//{
			//	LogInfo("Pts: %lld, Dts: %lld, Duration: %lld\n", 
			//			Packet->pts, Packet->dts,  Packet->duration);
			//}
			Stream = m_InputContext->FindStream(Packet->stream_index);
			if (!Stream ||
				Stream->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_UNKNOWN ||
				Stream->codecpar->codec_id == AVCodecID::AV_CODEC_ID_NONE)
			{
				continue;
			}

			eStreamType = kStreamIndex.at(Stream->codecpar->codec_type);
			if (vContextInfo[nContextIndex].nStreams[(int)eStreamType] == -1)
				continue;

			nKey = m_nCurrentPrefix + (int)eStreamType;

			if (!QueueItems[(int)eStreamType])
				QueueItems[(int)eStreamType] = m_Cache->GetBufferQueue(nKey);
			if (!QueueItems[(int)eStreamType]) continue;

			// Check weather this packet is during the section
			if (m_dSectionFrom > 0)
			{
				dSecond = Packet->pts * av_q2d(Stream->time_base);

				if (dSecond < m_dSectionFrom || 
					(m_dSectionTo > 0 && dSecond > m_dSectionTo))
				{
					continue;
				}

				nPts = (int64_t)(m_dSectionFrom /
					av_q2d(Stream->time_base));

				Packet->pts -= nPts;
				Packet->dts -= nPts;
				Packet->pos = -1;
			}

			if (m_OutputContext)
			{
				av_packet_rescale_ts(Packet, Stream->time_base,
					mOutputCodecContext->at(eStreamType).m_Context->time_base);
			}

			//LogInfo("Current size: %zd, Stream index: %d.\n", 
			//			m_Cache->Size(nKey), Packet->stream_index);

			//ConsumeCache(nKey);
			QueueItems[(int)eStreamType]->Push(Packet);

			// Current Packet is push into queue, so we need a new one
			Packet = av_packet_alloc();

			StageSleep();
		}

		// Sometimes it will be stopped manual, eg: recording
		m_Cache->PushNullPacket(m_nCurrentPrefix);

		if (Packet) av_packet_free(&Packet);

		// For recording, close device context while exit
		if (m_InputContext->m_Context->iformat)
			avformat_close_input(&m_InputContext->m_Context);

		Thread::Run();
	}

	void CDemuxer::Stop()
	{
		if (m_InputContext && m_bIsInputCtxEmpty && !IsStop())
		{
			// If it's an empty input file, then auto write the end frame
			auto& 	vContextInfo = m_Cache->GetContextInfos();
			int 	nContextIndex = GetContextIndex();

			if (nContextIndex < vContextInfo.size())
			{
				for (int i = 0; i < (int)EStreamType::EST_Max; i++)
				{
					if (vContextInfo[nContextIndex].nStreams[i] != -1)
					{
						WriteFrameDatas((EStreamType)i, nullptr, 0);
					}
				}
			}
		}

		Thread::Stop();
	}

	AVFrame* CDemuxer::WriteVideoFrame(AVCodecContext* n_CodecContext, 
		const void* n_Data, const int& n_nSize)
	{
		if (!m_funcFillVideoFrame) return nullptr;
		
		AVFrame* Frame = FFrame::VideoFrame(n_CodecContext->width,
			n_CodecContext->height, n_CodecContext->pix_fmt);
		
		if (Frame)
		{
			m_funcFillVideoFrame(Frame, n_Data, n_nSize);
			Frame->duration = 1;
			Frame->pts = m_nVideoFrameIndex++;
		}

		return Frame;
	}

	AVFrame* CDemuxer::WriteAudioFrame(AVCodecContext* n_CodecContext,
		const void* n_Data, const int& n_nSize)
	{
		if (!m_funcFillAudioFrame) return nullptr;
		
		AVFrame* Frame = FFrame::AudioFrame(
			n_CodecContext->frame_size, n_CodecContext->sample_rate,
			n_CodecContext->sample_fmt, &n_CodecContext->ch_layout);

		if (Frame)
		{
			m_funcFillAudioFrame(Frame, n_Data, n_nSize);
			Frame->pts = m_nAudioFramePts;
			m_nAudioFramePts += Frame->nb_samples;
		}

		return Frame;
	}

	void CDemuxer::SetDuration()
	{
		double dDuration = m_InputContext->Duration();

		if (m_dSectionFrom > 0)
		{
			if (m_dSectionTo > 0)
				dDuration = m_dSectionTo - m_dSectionFrom;
			else
				dDuration = dDuration - m_dSectionFrom;
		}

		int nContextIndex = GetContextIndex();
		m_Cache->SetContextDuration(nContextIndex, dDuration);
	}

}
