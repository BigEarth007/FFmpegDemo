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

	void CDemuxComponent::Init(int n_nContextIndex,
		const std::map<EStreamType, int64_t>& n_mPts)
	{
		IComponent::Init(n_nContextIndex);

		m_nContextIndex = n_nContextIndex;
		auto mOutputCodecContext = m_OutputContext->GetCodecContext();

		CInputContext* InputContext = GetInputContext();
		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			EStreamType Type = EStreamType(i);

			if (InputContext->GetStreamIndex((EStreamType)i) != -1)
			{
				// Set timebase of the stream
				auto itr1 = mOutputCodecContext->find(Type);
				if (itr1 != mOutputCodecContext->end())
				{
					m_mStreamInfo[Type].TimeBase = itr1->second.m_Context->time_base;
				}
				else
				{
					auto MediaType = StreamType2MediaType(Type);
					auto Stream = InputContext->GetContext().FindStream(MediaType);
					if (Stream) m_mStreamInfo[Type].TimeBase = Stream->time_base;
				}

				// Set pts of last input context
				auto itr2 = n_mPts.find(Type);
				if (itr2 != n_mPts.end())
					m_mStreamInfo[Type].LastMax = itr2->second;

				SetStreamEndFlag((EStreamType)i, 0);

				// Convert the selected sections from timestamps to pts;
				if (m_mStreamInfo[Type].TimeBase.den == 1) continue;
				auto rational = av_q2d(m_mStreamInfo[Type].TimeBase);

				auto& vSections = InputContext->GetSelectedSections();
				for (size_t j = 0; j < vSections.size(); j++)
				{
					int64_t pts = (int64_t)(vSections[j].dFrom / rational);
					m_mStreamInfo[Type].Sections.push_back(pts);
					pts = (int64_t)(vSections[j].dTo / rational);
					m_mStreamInfo[Type].Sections.push_back(pts);
				}
			}
		}
	}

	int CDemuxComponent::Run()
	{
		ThrowExceptionExpr(m_nContextIndex < 0, "Call function SetInputContext please!\n");

		if (GetEndFlag()) return AVERROR_EOF;
		CInputContext*	InputContext = GetInputContext();

		if (!InputContext->IsValid()) return AVERROR_EOF;

		int				ret = 0;
		int64_t			nDiscardDuration = 0;
		AVStream*		Stream = nullptr;
		AVPacket*		Packet = av_packet_alloc();
		EStreamType		eStreamType = EStreamType::ST_Size;

		do
		{
			ret = InputContext->GetContext().ReadPacket(Packet);
			if (ret < 0)
			{
				WriteEndData();
				AVDebug("-----------------------------------------------------\n\n");
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
			if (InputContext->GetStreamIndex(eStreamType) == -1 ||
				0 != GetStreamEndFlag(eStreamType))
			{
				// Current stream is not use, or is end
				break;
			}

			auto itr = m_mStreamInfo.find(eStreamType);
			if (itr == m_mStreamInfo.end()) break;

			Packet->pts = Packet->dts;

			if (itr->second.TimeBase.den > 1)
			{
				av_packet_rescale_ts(Packet, Stream->time_base,
					itr->second.TimeBase);
			}

			nDiscardDuration = itr->second.IsPacketUseable(
				Packet->pts, Packet->duration);
			if (nDiscardDuration == -1) break;
			else if (nDiscardDuration == AVERROR_EOF)
			{
				WriteData(eStreamType, nullptr,
					EDataType::DT_Packet, InputContext->GetSubnumber());
				SetStreamEndFlag(eStreamType, 1);
				break;
			}

			Packet->pts += itr->second.LastMax - nDiscardDuration;
			Packet->dts = Packet->pts;
			Packet->pos = -1;

			if (itr->second.CurMax < Packet->pts + Packet->duration)
				itr->second.CurMax = Packet->pts + Packet->duration;

			//LogInfo("Current size: %zd, Stream index: %d.\n", 
			//			m_Cache->Size(nKey), Packet->stream_index);
			//AVDebug("StreamIndex: %d; pts: %zd => dts: %zd======\n", Packet->stream_index, Packet->pts, Packet->dts);

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
		if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);
		return WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
	}

	CInputContext* CDemuxComponent::GetInputContext()
	{
		ThrowExceptionExpr(!m_Editor, "Call function SetEditor please!\n");

		return m_Editor->GetInputContext(m_nContextIndex);
	}

	const std::map<aveditor::EStreamType, int64_t> CDemuxComponent::GetPts() const
	{
		std::map<aveditor::EStreamType, int64_t> m;

		for (auto itr = m_mStreamInfo.begin(); itr != m_mStreamInfo.end(); itr++)
		{
			m[itr->first] = itr->second.CurMax;
		}

		return m;
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
				SetStreamEndFlag((EStreamType)i, 1);
			}
		}
	}

	int64_t CDemuxComponent::FStreamInfo::IsPacketUseable(const int64_t n_nPts, const int64_t n_nDuration)
	{
		bool selected = true;

		size_t i = 0;
		int64_t start = 0;
		int64_t nDiscardDuration = 0;

		// There is no selected section
		if (Sections.size() == 0) return nDiscardDuration;

		for (i = 0; i < Sections.size(); i+=2)
		{
			if (n_nPts < Sections[i])
			{
				if (n_nPts + n_nDuration > Sections[i])
					Sections[i] = n_nPts + n_nDuration;
				selected = false;
				break;
			}

			if (i > 0) start = Sections[i - 1];
			nDiscardDuration += Sections[i] - start;

			if (n_nPts < Sections[i + 1]) break;
		}

		// It is end of selected section
		if (i >= Sections.size()) return AVERROR_EOF;
		// Not in selected sections
		if (!selected) return -1;
		// In the selected sections, adjust pts
		return nDiscardDuration;
	}

}
