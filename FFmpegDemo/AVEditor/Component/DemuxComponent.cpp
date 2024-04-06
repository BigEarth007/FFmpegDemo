#include "pch.h"
#include "DemuxComponent.h"


namespace aveditor
{
	CDemuxComponent::CDemuxComponent()
	{
		SetCompID(ECompID::EI_Demux);
	}

	CDemuxComponent::~CDemuxComponent()
	{
		Release();
	}

	void CDemuxComponent::Init(int n_nContextIndex)
	{
		IComponent::Init(n_nContextIndex);

		m_nContextIndex = n_nContextIndex;
		m_OutputCodecContext = m_OutputContext->GetCodecContext();
	}

	int CDemuxComponent::Run()
	{
		ThrowExceptionExpr(m_nContextIndex < 0, "Call function SetInputContext please!\n");

		if (GetEndFlag()) return AVERROR_EOF;
		CInputContext*	InputContext = GetInputContext();

		if (!InputContext->IsValid()) return AVERROR_EOF;

		int				ret = 0;
		int64_t			nPts = 0;
		double			dSecond = 0;
		AVStream*		Stream = nullptr;
		AVPacket*		Packet = av_packet_alloc();
		EStreamType		eStreamType = EStreamType::ST_Size;

		do
		{
			ret = InputContext->GetContext().ReadPacket(Packet);
			if (ret < 0)
			{
				WriteEndData();
				SetEndFlag(true);
				break;
			}

			//if (Packet->stream_index == 0)
			//{
			//	LogInfo("Pts: %lld, Dts: %lld, Duration: %lld\n", 
			//			Packet->pts, Packet->dts,  Packet->duration);
			//}
			Stream = InputContext->GetContext().FindStream(Packet->stream_index);
			if (!Stream ||
				Stream->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_UNKNOWN ||
				Stream->codecpar->codec_id == AVCodecID::AV_CODEC_ID_NONE)
			{
				break;
			}

			eStreamType = MediaType2StreamType(Stream->codecpar->codec_type);
			if (InputContext->GetStreamIndex(eStreamType) == -1)
			{
				break;
			}

			// Check weather this packet is during the section
			if (InputContext->GetSectionFrom() > 0)
			{
				dSecond = Packet->pts * av_q2d(Stream->time_base);

				if (dSecond < InputContext->GetSectionFrom() ||
					(InputContext->GetSectionTo() > 0 &&
						dSecond > InputContext->GetSectionTo()))
				{
					break;
				}

				nPts = (int64_t)(InputContext->GetSectionFrom() /
					av_q2d(Stream->time_base));

				Packet->pts -= nPts;
				Packet->dts -= nPts;
				Packet->pos = -1;
			}

			auto itr = m_OutputCodecContext->find(eStreamType);
			if (itr != m_OutputCodecContext->end())
			{
				av_packet_rescale_ts(Packet, Stream->time_base,
					itr->second.m_Context->time_base);
			}

			//LogInfo("Current size: %zd, Stream index: %d.\n", 
			//			m_Cache->Size(nKey), Packet->stream_index);

			// Limit buffer size to reduce memory usage
			if (m_AVIOHandle && 
				m_AVIOHandle->GetBufferSize(eStreamType) > m_nMaxBufferSize)
			{
				size_t nSize = m_nMaxBufferSize;
				for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
				{
					// get min buffer size of other stream buffer
					EStreamType st = (EStreamType)i;
					if (InputContext->GetStreamIndex(st) != -1 &&
						st != eStreamType &&
						m_AVIOHandle->GetBufferSize(st) < nSize)
					{
						nSize = m_AVIOHandle->GetBufferSize(st);
					}
				}

				m_AVIOHandle->EnableBufferSizeLimited(nSize > 0);
			}

			WriteData(eStreamType, Packet,
				EDataType::DT_Packet, InputContext->GetSubnumber());

			Packet = nullptr;

		} while (0);

		if (!Packet) av_packet_free(&Packet);

		return ret;
	}

	void CDemuxComponent::Release()
	{
		IComponent::Release();
	}

	int CDemuxComponent::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		return WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
	}

	CInputContext* CDemuxComponent::GetInputContext()
	{
		ThrowExceptionExpr(!m_Editor, "Call function SetEditor please!\n");

		return m_Editor->GetInputContext(m_nContextIndex);
	}

	void CDemuxComponent::WriteEndData()
	{
		CInputContext* InputContext = GetInputContext();

		EDataType eDataType = EDataType::DT_Packet;
		if (!InputContext->IsValid())
			eDataType = EDataType::DT_Frame;

		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			if (InputContext->GetStreamIndex((EStreamType)i) != -1)
			{
				WriteData((EStreamType)i, nullptr,
					eDataType, InputContext->GetSubnumber());
			}
		}
	}

}