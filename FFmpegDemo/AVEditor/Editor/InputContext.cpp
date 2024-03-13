#include "pch.h"
#include "InputContext.h"


namespace aveditor
{
	CInputContext::CInputContext(std::vector<IStage*>& n_vStages, 
		FCache& n_Cache, const int n_nContextIndex)
		: CBaseContext(n_vStages, n_Cache, n_nContextIndex)
	{

	}

	CInputContext::~CInputContext()
	{

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

	CDemuxer* CInputContext::CreateDemuxer(FFormatContext& n_OutputContext)
	{
		FContextInfo* ContextInfo = m_Cache->GetContextInfo(m_nContextIndex);
		if (!ContextInfo) return nullptr;

		EItemType ItemType = EItemType::EIT_Packet;
		// For empty input file, no packet can be read
		if (!m_Context.m_Context) ItemType = EItemType::EIT_Frame;

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (ContextInfo->nStreams[i] == -1) continue;

			m_Cache->CreateCache(EStage::ES_Demux,
				ItemType, m_nContextIndex, (EStreamType)i);
		}

		int nPrefix = StageToPrefix(EStage::ES_Demux, m_nContextIndex);

		CDemuxer* Demuxer = new CDemuxer(*m_Cache, nPrefix);
		Demuxer->Init(m_Context, &n_OutputContext);

		m_vStages->emplace_back(Demuxer);

		return Demuxer;
	}

	void CInputContext::CreateDecoder(FFormatContext& n_OutputContext)
	{
		FContextInfo* ContextInfo = m_Cache->GetContextInfo(m_nContextIndex);
		if (!ContextInfo) return;

		std::map<EStreamType, FCodecContext>* mInputCodecContext = m_Context.GetCodecContext();
		std::map<EStreamType, FCodecContext>* mOutputCodecContext = n_OutputContext.GetCodecContext();

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (ContextInfo->nStreams[i] == -1) continue;
			if (ContextInfo->eJob == EJob::EJ_AMixBranch &&
				i == (int)EStreamType::EST_Video)
				continue;

			auto itrInput = mInputCodecContext->find((EStreamType)i);
			if (itrInput == mInputCodecContext->end()) continue;

			auto itrOutput = mOutputCodecContext->find((EStreamType)i);
			if (itrOutput == mOutputCodecContext->end()) continue;

			// if n_OutputContext.m_Context == nullptr, it's playing video
			// The same codec type, no need to decode
			if (n_OutputContext.m_Context &&
				itrInput->second.m_Context->codec_id ==
				itrOutput->second.m_Context->codec_id &&
				ContextInfo->eJob == EJob::EJ_Normal)
				continue;

			int nPrefix = m_Cache->CreateCache(EStage::ES_Decode,
				EItemType::EIT_Frame, m_nContextIndex, (EStreamType)i);
			CDecoder* Decoder = new CDecoder(*m_Cache, nPrefix, (EStreamType)i);
			Decoder->Init(itrInput->second, itrOutput->second);

			m_vStages->emplace_back(Decoder);
		}
	}

	void CInputContext::CreateEncoder(FFormatContext& n_OutputContext)
	{
		FContextInfo* ContextInfo = m_Cache->GetContextInfo(m_nContextIndex);
		if (!ContextInfo) return;

		std::map<EStreamType, FCodecContext>* mInputCodecContext = m_Context.GetCodecContext();
		std::map<EStreamType, FCodecContext>* mOutputCodecContext = n_OutputContext.GetCodecContext();

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (ContextInfo->nStreams[i] == -1) continue;
			if (ContextInfo->eJob == EJob::EJ_AMixBranch &&
				i == (int)EStreamType::EST_Audio)
				continue;

			auto itrInput = mInputCodecContext->find((EStreamType)i);
			if (itrInput == mInputCodecContext->end()) continue;

			auto itrOutput = mOutputCodecContext->find((EStreamType)i);
			if (itrOutput == mOutputCodecContext->end()) continue;

			// The same codec type, no need to encode
			if (itrInput->second.m_Context->codec_id ==
				itrOutput->second.m_Context->codec_id &&
				ContextInfo->eJob == EJob::EJ_Normal)
				continue;

			int nPrefix = m_Cache->CreateCache(EStage::ES_Encode,
				EItemType::EIT_Packet, m_nContextIndex, (EStreamType)i);

			CEncoder* Encoder = new CEncoder(*m_Cache, nPrefix, (EStreamType)i);
			Encoder->Init(itrOutput->second.m_Context);

			m_vStages->emplace_back(Encoder);
		}
	}

	void CInputContext::CreateTranscoder(FFormatContext& n_OutputContext)
	{
		FContextInfo* ContextInfo = m_Cache->GetContextInfo(m_nContextIndex);
		if (!ContextInfo) return;

		std::map<EStreamType, FCodecContext>* mInputCodecContext = m_Context.GetCodecContext();
		std::map<EStreamType, FCodecContext>* mOutputCodecContext = n_OutputContext.GetCodecContext();

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (ContextInfo->nStreams[i] == -1) continue;

			auto itrInput = mInputCodecContext->find((EStreamType)i);
			if (itrInput == mInputCodecContext->end()) continue;

			auto itrOutput = mOutputCodecContext->find((EStreamType)i);
			if (itrOutput == mOutputCodecContext->end()) continue;

			// The same codec type, no need to decode/encode
			if (itrInput->second.m_Context->codec_id ==
				itrOutput->second.m_Context->codec_id &&
				ContextInfo->eJob == EJob::EJ_Normal)
				continue;

			int nPrefix = m_Cache->CreateCache(EStage::ES_Encode,
				EItemType::EIT_Packet, m_nContextIndex, (EStreamType)i);
			CTranscoder* Transcoder = new CTranscoder(*m_Cache, nPrefix, (EStreamType)i);
			Transcoder->Init(itrInput->second, itrOutput->second);

			m_vStages->emplace_back(Transcoder);
		}
	}

	IPlayer* CInputContext::CreatePlayer(IPlayer* n_Player)
	{
		FContextInfo* ContextInfo = m_Cache->GetContextInfo(m_nContextIndex);
		int nPrefix = m_Cache->CreateCache(EStage::ES_Decode, m_nContextIndex);
		int nFrameDuration = 40;

		AVStream* Stream = m_Context.FindStream(AVMediaType::AVMEDIA_TYPE_VIDEO);
		if (Stream)
			nFrameDuration = 1000 / Stream->avg_frame_rate.num;

		if (!n_Player)
		{
			CSDLPlayer* Player = new CSDLPlayer(*m_Cache, nPrefix);
			Player->Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO, nFrameDuration);
			n_Player = Player;
		}
		else
		{
			n_Player->BaseInit(*m_Cache, nPrefix, EStreamType::EST_Max);
		}

		m_vStages->emplace_back(n_Player);

		return n_Player;
	}

}
