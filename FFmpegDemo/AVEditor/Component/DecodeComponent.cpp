#include "pch.h"
#include "DecodeComponent.h"

namespace aveditor
{
	CAVConverter::CAVConverter()
	{
	}

	CAVConverter::~CAVConverter()
	{
		Release();
	}

	void CAVConverter::Init(FCodecContext* n_InputCodec,
		FCodecContext* n_OutputCodec,
		CEditor* n_Editor)
	{
		ThrowExceptionExpr(!n_InputCodec || !n_InputCodec->m_Context,
			"Missing input codec.\n");

		m_InputCodec = n_InputCodec;
		m_OutputCodec = n_OutputCodec;
		m_Editor = n_Editor;

		CompCodecFormat();
	}

	void CAVConverter::Release()
	{
		if (m_FrameConvert)
		{
			m_FrameConvert->Release();
			delete m_FrameConvert;
			m_FrameConvert = nullptr;
		}
	}

	int CAVConverter::Coverting(const EDataType n_eDataType, void* n_Data)
	{
		int ret = 0;

		switch (n_eDataType)
		{
		case EDataType::DT_Packet:
			ret = Decoding((AVPacket*)n_Data);
			break;
		case EDataType::DT_Frame:
			ret = ConvertFrame((AVFrame*)n_Data);
			break;
		default:
			break;
		}

		return ret;
	}

	int CAVConverter::ConvertFrame(AVFrame* n_Frame)
	{
		int ret = 0;

		if (m_FrameConvert)
		{
			if (!n_Frame)
			{
				m_FrameConvert->CleanCache();
				// Null frame for ending
				FinishedConvert(nullptr);
			}
			else
			{
				ret = m_FrameConvert->Process(n_Frame);
				if (ret != 0) av_frame_free(&n_Frame);
			}
		}

		return ret;
	}

	int CAVConverter::Decoding(AVPacket* n_Packet)
	{
		int ret = 0;

		if (!m_bNeedDecode)
		{
			if (m_Callback)
				ret = m_Callback(m_eStreamType, n_Packet,
					EDataType::DT_Packet, m_nSubnumber);
		}
		else
		{
			ret = m_InputCodec->DecodePacket(n_Packet,
				[this](AVFrame* n_Frame) {

					if (m_OutputCodec &&
						m_OutputCodec->m_Context &&
						n_Frame->pict_type == AVPictureType::AV_PICTURE_TYPE_B &&
						(m_OutputCodec->m_Context->codec->capabilities & AV_CODEC_CAP_DELAY) != 0)
					{
						// converter b frame to p frame
						n_Frame->pict_type = AVPictureType::AV_PICTURE_TYPE_P;
					}

					// if (n_Frame->pts <= 0 &&
					// 	n_CodecContext.m_Context->codec_id == AVCodecID::AV_CODEC_ID_MP3 &&
					// 	n_CodecContext.m_Context->frame_size > n_Frame->nb_samples)
					// {
					// 	return 1;
					// }

					// LogInfo("Decode frame: %d, PTS: %lld.\n", n_nCurrentKey, n_Frame->pts);
					n_Frame->pts = n_Frame->best_effort_timestamp;
					int ret = m_FrameConvert->Process(n_Frame);

					return ret;
				});

			if (!n_Packet) av_packet_free(&n_Packet);

			if (AVERROR_EOF == ret)
			{
				if (m_InputCodec->m_Context->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
				{
					if (m_Editor->GetCurrentBatchIndex() < m_Editor->GetMaxBatchIndex())
						m_Editor->SetAudioPts(m_FrameConvert->GetConvertPts());
					else
						m_FrameConvert->CleanCache();
				}
				m_FrameConvert->CleanCache();
				// Null frame for ending
				FinishedConvert(nullptr);
			}
		}

		return ret;
	}

	bool CAVConverter::IsConvertValid()
	{
		return m_FrameConvert && m_FrameConvert->IsValid();
	}

	void CAVConverter::SetSubnumber(const int n_nSubnumber)
	{
		m_nSubnumber = n_nSubnumber;
	}

	const int CAVConverter::GetSubnumber() const
	{
		return m_nSubnumber;
	}

	void CAVConverter::SetDecodeFlag(const bool n_bDecode)
	{
		m_bNeedDecode = n_bDecode;
	}

	const bool CAVConverter::GetDecodeFlag() const
	{
		return m_bNeedDecode;
	}

	void CAVConverter::SetFinishedCallback(CompCallback n_Callback)
	{
		m_Callback = n_Callback;
	}

	const int64_t CAVConverter::GetFramePts() const
	{
		return m_FrameConvert ? m_FrameConvert->GetConvertPts() : 0;
	}

	int CAVConverter::CompCodecFormat()
	{
		// The convert is valid if it's different between 2 codec
		if (m_InputCodec->m_Context->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			m_FrameConvert = new CVideoConvert();
			m_eStreamType = EStreamType::ST_Video;
		}
		else if (m_InputCodec->m_Context->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			m_FrameConvert = new CAudioConvert();
			m_FrameConvert->SetAudioFifo(m_Editor->GetAudioFifo());
			// Set audio pts of the last input context
			m_FrameConvert->SetConvertPts(m_Editor->GetAudioPts());
			m_eStreamType = EStreamType::ST_Audio;
		}

		ThrowExceptionExpr(!m_FrameConvert, "Could not create converter.\n");

		// Output context is empty, not need to covert, just decoding
		if (!m_OutputCodec || !m_OutputCodec->m_Context) return -1;

		ThrowExceptionExpr(
			m_InputCodec->m_Context->codec_type !=
			m_OutputCodec->m_Context->codec_type,
			"Codec type of input and output context is not match.\n");

		m_FrameConvert->Init(m_InputCodec, m_OutputCodec);

		m_FrameConvert->SetFinishedCallback(
			std::bind(&CAVConverter::FinishedConvert,
				this, std::placeholders::_1));

		return 0;
	}

	int CAVConverter::FinishedConvert(AVFrame* n_Frame)
	{
		if (m_OutputCodec && n_Frame)
		{
			n_Frame->sample_rate = m_OutputCodec->m_Context->sample_rate;
			n_Frame->time_base = m_OutputCodec->m_Context->time_base;
		}

		int ret = 0;
		if (m_Callback)
			ret = m_Callback(m_eStreamType, n_Frame,
				EDataType::DT_Frame, m_nSubnumber);

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////

	CDecodeComponent::CDecodeComponent()
	{
		SetCompID(ECompID::EI_Decode);
	}

	CDecodeComponent::~CDecodeComponent()
	{
		Release();
	}

	void CDecodeComponent::Init(int n_nContextIndex)
	{
		IComponent::Init(n_nContextIndex);

		bool bDecodeFlag[(int)EStreamType::ST_Size] = { false };

		auto InputContext = m_Editor->GetInputContext(n_nContextIndex);
		auto mOutputCodecContext = m_OutputContext->GetCodecContext();
		auto mInputCodecContext = InputContext->GetContext().GetCodecContext();

		if (InputContext->IsValid())
		{
			if (!m_OutputContext->IsValid())
			{
				// Output context is invalid or need to filter frame
				// Then the packet should be decoded
				for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
				{
					bDecodeFlag[i] = true;
				}
			}
			else if (InputContext->GetTask() == ETask::T_AMixMain ||
				InputContext->GetTask() == ETask::T_AMixBranch)
			{
				bDecodeFlag[(int)EStreamType::ST_Audio] = true;
			}
		}

		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			// Check if packet should be decoded
			if (InputContext->GetStreamIndex((EStreamType)i) != -1)
			{
				auto itr = mInputCodecContext->find((EStreamType)i);
				ThrowExceptionExpr(itr == mInputCodecContext->end(),
					"Miss codec [%d] in input context.\n", i);

				FCodecContext* OutputCodecContext = nullptr;
				auto itr2 = mOutputCodecContext->find((EStreamType)i);
				if (itr2 != mOutputCodecContext->end())
					OutputCodecContext = &itr2->second;

				m_Decoder[i].Init(&itr->second, OutputCodecContext, m_Editor);
				m_Decoder[i].SetSubnumber(InputContext->GetSubnumber());

				m_Decoder[i].SetFinishedCallback(
					std::bind(&CDecodeComponent::FinishedDecode, this,
						std::placeholders::_1, std::placeholders::_2,
						std::placeholders::_3, std::placeholders::_4));

				// Converter is valid, so need to decode video packet
				bDecodeFlag[i] |= m_Decoder[i].IsConvertValid();
			}
		}

		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_Decoder[i].SetDecodeFlag(bDecodeFlag[i]);
			// no need to decode, set to end
			if (bDecodeFlag[i]) SetStreamEndFlag((EStreamType)i, 0);
		}
	}

	int CDecodeComponent::Run(EStreamType n_eStreamType)
	{
		int ret = 0;

		if (GetEndFlag()) return AVERROR_EOF;

		EDataType eDataType = EDataType::DT_None;
		void* pData = nullptr;

		if (GetStreamEndFlag(n_eStreamType) == 0)
		{
			ret = Pop(n_eStreamType, eDataType, pData, kSleepDelay);
			if (ret >= 0)
			{
				ret = m_Decoder[(int)n_eStreamType].Coverting(eDataType, pData);
			}
		}

		return ret;
	}

	void CDecodeComponent::Release()
	{
		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_Decoder[i].Release();
		}

		IComponent::Release();
	}

	int CDecodeComponent::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (m_Decoder[(int)n_eStreamType].GetDecodeFlag())
		{
			ret = IComponent::ReceiveData(n_eStreamType, 
				n_Data, n_eType, n_nIndex);
		}
		else
		{
			if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);
			ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
		}

		return ret;
	}

	int CDecodeComponent::FinishedDecode(const EStreamType n_eStreamType,
		void* n_Data, EDataType n_eType, int n_nIndex)
	{
		if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);

		int ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);

		return ret;
	}

}
