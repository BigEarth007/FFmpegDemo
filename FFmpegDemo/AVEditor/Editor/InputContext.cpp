#include "pch.h"
#include "InputContext.h"


namespace aveditor
{
	CInputContext::CInputContext(CEditor& n_Editor, const int n_nContextIndex)
		: CBaseContext(n_Editor, n_nContextIndex)
	{
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

	const double CInputContext::Length()
	{
		double dLength = 0;

		for (size_t i = 0; i < m_vSections.size(); i++)
		{
			dLength += m_vSections[i].dTo - m_vSections[i].dFrom;
		}

		if (dLength == 0) dLength = m_Context.Length();

		return dLength;
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
				if (n_nStreams & (1 << i)) m_nStreams[i] = i;
			}
		}
		else
		{
			// It's a valid input file
			AVStream* Stream = nullptr;

			for (int i = 0;i < (int)EStreamType::ST_Size; i++)
			{
				m_nStreams[i] = -1;
				if (n_nStreams & (1 << i))
				{
					AVMediaType eMediaType = StreamType2MediaType((EStreamType)i);

					if (eMediaType == AVMediaType::AVMEDIA_TYPE_UNKNOWN)
						continue;

					Stream = m_Context.FindStream(eMediaType);
					if (Stream) m_nStreams[i] = Stream->index;
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

	void CInputContext::AddSelectedSection(const double n_dStart, 
		const double n_dLength /*= 0*/)
	{
		double dLength = m_Context.Length();
		if (n_dStart >= dLength) return;

		FSection Section;

		Section.dFrom = n_dStart;
		if (Section.dFrom < 0) Section.dFrom = 0;

		Section.dTo = Section.dFrom + n_dLength;
		if (Section.dTo > dLength || n_dLength == 0) 
			Section.dTo = dLength;

		if (m_vSections.size() == 0)
		{
			m_vSections.emplace_back(Section);
		}
		else
		{
			int nFlag = 0;
			std::vector<FSection> vTemp(m_vSections.begin(), m_vSections.end());
			m_vSections.clear();

			for (size_t i = 0; i < vTemp.size(); i++)
			{
				if (nFlag == 0)
				{
					if (vTemp[i].dFrom > Section.dFrom)
					{
						nFlag++;
						i--;
					}
					else if (vTemp[i].dTo > Section.dFrom)
					{
						Section.dFrom = vTemp[i].dFrom;
						nFlag++;
						i--;
					}
					else
					{
						m_vSections.emplace_back(vTemp[i]);
						if (i + 1 == vTemp.size())
							m_vSections.emplace_back(Section);
					}
				}
				else if (nFlag == 1)
				{
					if (vTemp[i].dFrom >= Section.dTo)
					{
						nFlag++;
						m_vSections.emplace_back(Section);
						m_vSections.emplace_back(vTemp[i]);
					}
					else if (vTemp[i].dTo >= Section.dTo)
					{
						Section.dTo = vTemp[i].dTo;
						nFlag++;
						m_vSections.emplace_back(Section);
					}
					else if (vTemp[i].dTo < Section.dTo)
					{
						if (i + 1 == vTemp.size())
							m_vSections.emplace_back(Section);
					}
				}
				else
				{
					m_vSections.emplace_back(vTemp[i]);
				}
			}
		}
	}

	void CInputContext::RemoveSelectedSection(const size_t& n_nSectionIndex)
	{
		if (n_nSectionIndex < m_vSections.size())
			m_vSections.erase(m_vSections.begin() + n_nSectionIndex);
	}

	int64_t CInputContext::IsPacketInSelectedSection(AVPacket* n_Packet,
		const AVRational& n_TimeBase)
	{
		if (m_vSections.size() == 0) return -1;

		int64_t nPts = AVERROR_EOF;
		size_t i = 0;
		double dDuration = 0;
		double dSecond = n_Packet->pts * av_q2d(n_TimeBase);

		for (i = 0;i < m_vSections.size(); i++)
		{
			if (dSecond < m_vSections[i].dFrom)
				break;

			dDuration += m_vSections[i].dFrom;
			if (i > 0) dDuration -= m_vSections[i - 1].dTo;

			if (m_vSections[i].dFrom <= dSecond &&
				m_vSections[i].dTo > dSecond)
			{
				nPts = (int64_t)(dDuration / av_q2d(n_TimeBase));
				break;
			}
		}

		if (nPts != AVERROR_EOF)
		{
			n_Packet->pts -= nPts;
			n_Packet->dts -= nPts;
			n_Packet->pos = -1;
			return i;
		}
		if (i == m_vSections.size()) return nPts;

		return -2;
	}

	void CInputContext::WriteFrameDatas(EStreamType n_eStreamType,
		const void* n_Data, const int& n_nSize)
	{
		if (!m_Context.m_sName.empty() || !m_CtxHandle)
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

		m_CtxHandle->WriteFrameData(n_eStreamType,
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
