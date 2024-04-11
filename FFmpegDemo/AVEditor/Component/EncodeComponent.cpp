#include "pch.h"
#include "EncodeComponent.h"

namespace aveditor
{

	CEncodeComponent::CEncodeComponent()
	{
		SetCompID(ECompID::EI_Encode);
	}

	CEncodeComponent::~CEncodeComponent()
	{
		Release();
	}

	void CEncodeComponent::Init()
	{
		IComponent::Init();

		m_OutputCodecContext = m_OutputContext->GetOutputCodecContext();

		for (auto itr = m_OutputCodecContext->begin();
			itr != m_OutputCodecContext->end(); itr++)
		{
			SetStreamEndFlag(itr->first, 0);
		}
	}

	int CEncodeComponent::Run(EStreamType n_eStreamType)
	{
		int ret = 0;

		if (GetEndFlag()) 
			return AVERROR_EOF;

		EDataType eDataType = EDataType::DT_None;
		void* pData = nullptr;

		if (GetStreamEndFlag(n_eStreamType) == 0)
		{
			ret = Pop(n_eStreamType, eDataType, pData, kSleepDelay);
			if (ret >= 0 && eDataType == EDataType::DT_Frame)
			{
				ret = EncodeFrame(n_eStreamType, (AVFrame*)pData);
				if (ret == AVERROR_EOF) SetStreamEndFlag(n_eStreamType, 1);
			}
		}

		return ret;
	}

	void CEncodeComponent::Release()
	{
		if (m_OutputCodecContext)
		{
			for (auto itr = m_OutputCodecContext->begin();
				itr != m_OutputCodecContext->end(); itr++)
			{
				SetStreamEndFlag(itr->first, 0);
			}
		}

		IComponent::Release();
	}

	int CEncodeComponent::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (n_eType == EDataType::DT_Frame && m_OutputContext->IsValid())
		{
			// For a valid output context, frame should be encode
			ret = IComponent::ReceiveData(n_eStreamType, n_Data, n_eType, n_nIndex);
		}
		else
		{
			if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);
			ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
		}

		return ret;
	}

	int CEncodeComponent::EncodeFrame(EStreamType n_eStreamType, AVFrame* n_Frame)
	{
		int ret = 0;

		auto itr = m_OutputCodecContext->find(n_eStreamType);
		if (itr != m_OutputCodecContext->end())
		{
			ret = itr->second.EncodeFrame(n_Frame,
				[this, &n_eStreamType](AVPacket* n_Packet) {

					//LogInfo("StreamIndex: %d; Pts: %lld, Dts: %lld, Duration: %lld\n",
					//	n_Packet->stream_index, n_Packet->pts, n_Packet->dts, n_Packet->duration);

					if (n_Packet->pts < n_Packet->dts)
						n_Packet->pts = n_Packet->dts;

					WriteData(n_eStreamType, n_Packet, EDataType::DT_Packet, 0);

					return 0;
				}
			);

			av_frame_free(&n_Frame);

			if (ret == AVERROR_EOF)
				WriteData(n_eStreamType, nullptr, EDataType::DT_Packet, 0);
		}

		return ret;
	}

}
