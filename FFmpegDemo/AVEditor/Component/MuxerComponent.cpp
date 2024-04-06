#include "pch.h"
#include "MuxerComponent.h"

namespace aveditor
{
	CMuxerComponent::CMuxerComponent()
	{
		SetCompID(ECompID::EI_Muxer);
	}

	CMuxerComponent::~CMuxerComponent()
	{
		Release();
	}

	void CMuxerComponent::Init()
	{
		IComponent::Init();

		auto OutputCodecContext = m_OutputContext->GetCodecContext();

		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_TimeSync[i].nStatus = -1;
			m_TimeSync[i].nStream = i;
		}

		for (auto itr = OutputCodecContext->begin();
			itr != OutputCodecContext->end(); itr++)
		{
			m_TimeSync[(int)itr->first].dTimebase =
				av_q2d(itr->second.m_Context->time_base);
			m_TimeSync[(int)itr->first].nStatus = 0;
		}

		m_nStreamSize = (int)m_OutputContext->GetCodecContext()->size();

		for (int i = 0; i < m_nStreamSize; i++)
		{
			AVStream* Stream = m_OutputContext->FindStream(i);
			if (Stream)
			{
				EStreamType eType = MediaType2StreamType(
					Stream->codecpar->codec_type);
				m_TimeSync[(int)eType].nStream = i;
			}
		}
	}

	int CMuxerComponent::Run()
	{
		int		ret = 0;
		int64_t	nPts = 0;

		if (GetEndFlag() || !m_OutputContext->IsValid())
			return AVERROR_EOF;

		bool bWritable = ReadCache();
		if (m_nStreamSize == 0)
		{
			SetEndFlag(true);
			return AVERROR_EOF;
		}
		if (!bWritable) return ret;

		// The duration of audio stream should not large than video stream
		if (m_TimeSync[(int)EStreamType::ST_Video].nStatus == 1 &&
			m_TimeSync[(int)EStreamType::ST_Video].dTimestamp <
			m_TimeSync[m_nMinIndex].dTimestamp)
		{
			SetEndFlag(true);
			return AVERROR_EOF;
		}

		if (m_TimeSync[m_nMinIndex].Packet->pts < 0)
		{
			m_dDuration -= m_TimeSync[m_nMinIndex].dTimestamp;
			nPts -= m_TimeSync[m_nMinIndex].Packet->pts;
		}

		nPts = (int64_t)(m_dDuration / m_TimeSync[m_nMinIndex].dTimebase);
		m_TimeSync[m_nMinIndex].Packet->stream_index = m_TimeSync[m_nMinIndex].nStream;
		m_TimeSync[m_nMinIndex].Packet->pts += nPts;
		m_TimeSync[m_nMinIndex].Packet->dts += nPts;

		m_OutputContext->InterleavedWritePacket(m_TimeSync[m_nMinIndex].Packet);
		m_nReadSize--;

		av_packet_free(&m_TimeSync[m_nMinIndex].Packet);
		m_TimeSync[m_nMinIndex].Packet = nullptr;

		return 0;
	}

	void CMuxerComponent::Release()
	{
		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			if (m_TimeSync[i].Packet)
				av_packet_free(&m_TimeSync[i].Packet);
			m_TimeSync[i].Packet = nullptr;

			if (m_TimeSync[i].nStatus != -1)
				m_TimeSync[i].nStatus = 0;

			m_TimeSync[i].dTimestamp = 0;
		}

		m_nMinIndex = 0;
		m_nReadSize = 0;
		if (m_OutputContext)
			m_nStreamSize = (int)m_OutputContext->GetCodecContext()->size();

		IComponent::Release();
	}

	int CMuxerComponent::ReceiveData(const EStreamType n_eStreamType,
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (GetEndFlag()) return AVERROR_EOF;

		if (!m_OutputContext->IsValid() || n_eType == EDataType::DT_Frame)
		{
			// Invalid output context, send data outside
			ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);

			if (m_TimeSync[(int)n_eStreamType].Writable() && !n_Data)
			{
				m_TimeSync[(int)n_eStreamType].nStatus = 1;
				m_nStreamSize--;
			}

			if (m_nStreamSize == 0)
			{
				SetEndFlag(true);
			}
		}
		else if (n_eType == EDataType::DT_Packet)
		{
			ret = IComponent::ReceiveData(n_eStreamType, n_Data, n_eType, n_nIndex);
		}

		return ret;
	}

	void CMuxerComponent::AddDuration(const double n_dDuration)
	{
		m_dDuration += n_dDuration;
	}

	bool CMuxerComponent::ReadCache()
	{
		int ret = 0;

		EDataType eDataType = EDataType::DT_None;
		void* pData = nullptr;

		if (m_nStreamSize == 0) return ret;

		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			if (m_TimeSync[i].Writable())
			{
				// Read packet from queue
				ret = Pop((EStreamType)i, eDataType, pData, kSleepDelay);
				if (ret > -1)
				{
					if (!pData)
					{
						m_TimeSync[i].nStatus = 1;
						m_nStreamSize--;
					}
					else
					{
						m_TimeSync[i].Fill((AVPacket*)pData);
						m_nReadSize++;
					}
				}
			}
		}

		if (m_nStreamSize != 0 && m_nStreamSize == m_nReadSize)
		{
			m_nMinIndex = -1;
			for (int i = 0; i < (int)EStreamType::ST_Size; i++)
			{
				// It is not in use
				if (m_TimeSync[i].nStatus != 0) continue;

				if (m_nMinIndex == -1 ||
					m_TimeSync[m_nMinIndex].dTimestamp > m_TimeSync[i].dTimestamp)
				{
					m_nMinIndex = i;
				}
			}
		}

		return m_nStreamSize == m_nReadSize;
	}

}
