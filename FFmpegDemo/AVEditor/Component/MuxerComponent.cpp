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

		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			// Set default stream index
			m_TimeSync[i].nStreamIndex = i;
		}

		auto OutputCodecContext = m_OutputContext->GetCodecContext();

		for (auto itr = OutputCodecContext->begin();
			itr != OutputCodecContext->end(); itr++)
		{
			m_TimeSync[(int)itr->first].dTimestamp = -AV_NOPTS_VALUE;
			m_TimeSync[(int)itr->first].dTimebase =
				av_q2d(itr->second.m_Context->time_base);

			// Get stream index
			AVMediaType eMediaType = StreamType2MediaType(itr->first);
			AVStream* Stream = m_OutputContext->FindStream(eMediaType);
			if (Stream) m_TimeSync[(int)itr->first].nStreamIndex = Stream->index;

			SetStreamEndFlag(itr->first, 0);
		}
	}

	int CMuxerComponent::Run()
	{
		if (GetEndFlag() || !m_OutputContext->IsValid())
			return AVERROR_EOF;

		AVPacket* Packet = ReadCache();
		if (!Packet) return 0;
		if (GetEndFlag()) return AVERROR_EOF;

		m_OutputContext->InterleavedWritePacket(Packet);

		av_packet_free(&Packet);

		return 0;
	}

	void CMuxerComponent::Release()
	{
		if (m_OutputContext)
		{
			auto OutputCodecContext = m_OutputContext->GetCodecContext();

			for (auto itr = OutputCodecContext->begin();
				itr != OutputCodecContext->end(); itr++)
			{
				m_TimeSync[(int)itr->first].dTimestamp = -AV_NOPTS_VALUE;
				m_TimeSync[(int)itr->first].nHold = 0;
				SetStreamEndFlag(itr->first, 0);
			}
		}

		IComponent::Release();
	}

	int CMuxerComponent::ReceiveData(const EStreamType n_eStreamType,
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (GetEndFlag()) return AVERROR_EOF;

		if (!m_OutputContext->IsValid() || n_eType == EDataType::DT_Frame)
		{
			if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);
			// Invalid output context, send data outside
			ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
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
		Duration2Pts();
	}

	void CMuxerComponent::SetDuration(const double n_dDuration)
	{
		m_dDuration = 0;
		Duration2Pts();
	}

	AVPacket* CMuxerComponent::ReadCache()
	{
		AVPacket* Packet = nullptr;

		if (GetEndFlag()) return Packet;

		int			ret = 0;
		void*		pData = nullptr;
		double		dTimestamp = 0;
		EDataType	eDataType = EDataType::DT_None;

		// Index of stream with min timestamp
		int			nMinIndex = AVERROR_EOF;

		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			// Pass end stream
			if (GetStreamEndFlag((EStreamType)i) != 0) continue;

			if (m_TimeSync[i].nHold == 0)
			{
				// Get first AVPacket
				ret = Front((EStreamType)i, eDataType, pData);
				if (ret < 0) return nullptr;

				if (!pData)
				{
					// Last AVPacket of this stream
					SetStreamEndFlag((EStreamType)i, 1);
					// The duration of audio stream should not large than video stream
					if (i == (int)EStreamType::ST_Video)
						SetStreamEndFlag(EStreamType::ST_Audio, 1);
					continue;
				}

				Packet = (AVPacket*)pData;
				dTimestamp = Packet->pts * m_TimeSync[i].dTimebase;

				if (m_TimeSync[i].dTimestamp > dTimestamp)
				{
					// Error packet, free it
					Pop((EStreamType)i, eDataType, pData, kSleepDelay);
					if (Packet->buf) AVFreeData(eDataType, pData);
					return nullptr;
				}

				m_TimeSync[i].dTimestamp = dTimestamp;
				m_TimeSync[i].nHold = 1;
			}

			if (nMinIndex == AVERROR_EOF ||
				m_TimeSync[nMinIndex].dTimestamp > m_TimeSync[i].dTimestamp)
			{
				nMinIndex = i;
			}
		}

		// All streams are end
		if (GetEndFlag()) return nullptr;

		Pop((EStreamType)nMinIndex, eDataType, pData, kSleepDelay);
		Packet = (AVPacket*)pData;

		if (Packet->pts < 0) AddDuration(-m_TimeSync[nMinIndex].dTimestamp);

		Packet->stream_index = m_TimeSync[nMinIndex].nStreamIndex;
		Packet->pts += m_TimeSync[nMinIndex].nPts;
		Packet->dts += m_TimeSync[nMinIndex].nPts;

		m_TimeSync[nMinIndex].nHold = 0;

		return Packet;
	}

	void CMuxerComponent::Duration2Pts()
	{
		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			if (m_TimeSync[i].dTimebase != 0)
				m_TimeSync[i].nPts = (int64_t)(m_dDuration / m_TimeSync[i].dTimebase);
		}
	}

}
