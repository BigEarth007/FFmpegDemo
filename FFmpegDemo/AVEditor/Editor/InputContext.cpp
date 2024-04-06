#include "pch.h"
#include "InputContext.h"


namespace aveditor
{
	CInputContext::CInputContext(CEditor& n_Editor, const int n_nContextIndex)
		: CBaseContext(n_Editor, n_nContextIndex)
	{
		for (int i = 0;i < (int)EStreamType::ST_Size; i++)
		{
			m_nStreams[i] = -1;
		}
	}

	CInputContext::~CInputContext()
	{
		Release();
	}

	FFormatContext& CInputContext::OpenInputFile(const std::string& n_sFileName,
		const AVInputFormat* n_InputFormat /*= nullptr*/, 
		AVDictionary* n_Options /*= nullptr*/)
	{
		m_Context.Release();

		if (!n_sFileName.empty())
		{
			m_Context.OpenInputFile(n_sFileName, n_InputFormat, n_Options);
			m_Context.GetInputCodecContext();
			m_Context.OpenCodecContext();
		}

		return m_Context;
	}

	const double CInputContext::Duration()
	{
		double dDuration = m_dSectionTo - m_dSectionFrom;
		if (dDuration == 0) dDuration = m_Context.Duration();

		return dDuration;
	}

	const double CInputContext::GetSectionFrom() const
	{
		return m_dSectionFrom;
	}

	const double CInputContext::GetSectionTo() const
	{
		return m_dSectionTo;
	}

	const std::string CInputContext::GetName() const
	{
		return m_Context.m_sName;
	}

	void CInputContext::SetTask(const ETask n_eTask)
	{
		m_eTask = n_eTask;
	}

	const ETask CInputContext::GetTask() const
	{
		return m_eTask;
	}

	void CInputContext::MarkStream(EStreamType n_eStreamType, const int n_nStreamIndex)
	{
		m_nStreams[(int)n_eStreamType] = n_nStreamIndex;
	}

	void CInputContext::MarkStream(const int n_nStreams)
	{
		if (m_Context.m_sName.empty())
		{
			for (int i = 0; i < (int)EStreamType::ST_Size; i++)
			{
				m_nStreams[i] = i;
			}
		}
		else
		{
			// It's a valid input file
			AVStream* Stream = nullptr;

			for (unsigned int i = 0, j = 0; i < m_Context.m_Context->nb_streams; i++)
			{
				Stream = m_Context.m_Context->streams[i];
				if (Stream->codecpar->codec_type != AVMediaType::AVMEDIA_TYPE_UNKNOWN &&
					Stream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_NONE)
				{
					EStreamType eStreamType = MediaType2StreamType(
						Stream->codecpar->codec_type);
					if (n_nStreams & (1 << (int)eStreamType))
					{
						// The stream index of context should start from 0, but the 
						// index of stream that we need may not start from 0. 
						// For example, detach audio stream from input context, 
						// and then write into output context
						m_nStreams[(int)eStreamType] = j++;
					}
				}
			}
		}
	}

	void CInputContext::UnmarkStream(EStreamType n_eStreamType)
	{
		m_nStreams[(int)n_eStreamType] = -1;
	}

	const int CInputContext::GetStreamIndex(EStreamType n_eStreamType) const
	{
		return m_nStreams[(int)n_eStreamType];
	}

	const int CInputContext::StreamsCode() const
	{
		int ret = 0;

		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			if (m_nStreams[i] != -1)
				ret |= (1 << i);
		}

		return ret;
	}

	void CInputContext::SetSubnumber(const int n_nSubnumber)
	{
		m_nSubnumber = n_nSubnumber;
	}

	const int CInputContext::GetSubnumber() const
	{
		return m_nSubnumber;
	}

	void CInputContext::SetBatchIndex(const int n_nBatchIndex)
	{
		m_nBatchIndex = n_nBatchIndex;
	}

	const int CInputContext::GetBatchIndex() const
	{
		return m_nBatchIndex;
	}

	void CInputContext::SelectSection(const double n_dStart, 
		const double n_dDuration /*= 0*/)
	{
		double dDuration = m_Context.Duration();

		if (n_dStart > dDuration || n_dStart <= 0)
			return;

		m_dSectionFrom = n_dStart;
		if (n_dDuration > 0)
			m_dSectionTo = m_dSectionFrom + n_dDuration;
		else
			m_dSectionTo = dDuration;
	}

	void CInputContext::WriteFrameDatas(EStreamType n_eStreamType,
		const void* n_Data, const int& n_nSize)
	{
		if (!m_Context.m_sName.empty() || !m_CtxHandle || !m_AVIOHandle)
			return;

		AVFrame* Frame = nullptr;

		if (n_Data && n_nSize > 0)
		{
			AVCodecContext* CodecContext =
				m_Context.GetCodecContext(n_eStreamType);
			if (!CodecContext) return;

			if (n_eStreamType == EStreamType::ST_Video)
			{
				Frame = WriteVideoFrame(CodecContext, n_Data, n_nSize);
			}
			else if (n_eStreamType == EStreamType::ST_Audio)
			{
				Frame = WriteAudioFrame(CodecContext, n_Data, n_nSize);
			}
		}

		m_AVIOHandle->ReceiveData(n_eStreamType, 
			Frame, EDataType::DT_Frame, m_nSubnumber);
	}

	void CInputContext::Release()
	{
		CBaseContext::Release();
	}

	AVFrame* CInputContext::WriteVideoFrame(AVCodecContext* n_CodecContext,
		const void* n_Data, const int& n_nSize)
	{
		AVFrame* Frame = FFrame::VideoFrame(n_CodecContext->width,
			n_CodecContext->height, n_CodecContext->pix_fmt);

		if (Frame)
		{
			m_CtxHandle->FillVideoFrame(Frame, n_Data, n_nSize);
			Frame->duration = 1;
			Frame->pts = m_nVideoFrameIndex++;
		}

		return Frame;
	}

	AVFrame* CInputContext::WriteAudioFrame(AVCodecContext* n_CodecContext, 
		const void* n_Data, const int& n_nSize)
	{
		AVFrame* Frame = FFrame::AudioFrame(
			n_CodecContext->frame_size, n_CodecContext->sample_rate,
			n_CodecContext->sample_fmt, &n_CodecContext->ch_layout);

		if (Frame)
		{
			m_CtxHandle->FillAudioFrame(Frame, n_Data, n_nSize);
			Frame->pts = m_nAudioFramePts;
			m_nAudioFramePts += Frame->nb_samples;
		}

		return Frame;
	}

}
