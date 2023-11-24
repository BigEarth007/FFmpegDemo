#include "pch.h"
#include "OutputContext.h"


namespace aveditor
{
	COutputContext::COutputContext(std::vector<IStage*>& n_vStages, 
		FCache& n_Cache, const int n_nContextIndex)
		: CBaseContext(n_vStages, n_Cache, n_nContextIndex)
	{

	}

	COutputContext::~COutputContext()
	{
	}

	FFormatContext& COutputContext::AllocOutputFile(const std::string& n_sFileName, 
		const AVOutputFormat* n_OutputFormat /*= nullptr*/, 
		const char* n_szFormatName /*= nullptr*/)
	{
		m_Context.Release();

		m_Context.AllocOutputFile(n_sFileName, n_OutputFormat, n_szFormatName);

		return m_Context;
	}

	void COutputContext::OpenOutputFile()
	{
		if (m_Context.m_Context)
		{
			m_Context.OpenOutputFile();
			m_Context.WriteHeader();
		}
	}

	void COutputContext::CloseOutputFile()
	{
		if (m_Context.m_Context)
			m_Context.WriteTrailer();
	}

	void COutputContext::CreateEncoder()
	{
		AVStream* Stream = nullptr;
		EStreamType eStreamType = EStreamType::EST_Max;
		std::map<EStreamType, FCodecContext>* mOutputCodecContext = m_Context.GetCodecContext();

		for (unsigned int i = 0; i < m_Context.m_Context->nb_streams; i++)
		{
			Stream = m_Context.m_Context->streams[i];
			eStreamType = kStreamIndex.at(Stream->codecpar->codec_type);

			auto itr = mOutputCodecContext->find(eStreamType);
			if (itr == mOutputCodecContext->end()) continue;

			int nPrefix = m_Cache->CreateCache(EStage::ES_Encode,
				EItemType::EIT_Packet, 0, eStreamType);

			CEncoder* Encoder = new CEncoder(*m_Cache, nPrefix, eStreamType);
			Encoder->Init(itr->second.m_Context);

			m_vStages->emplace_back(Encoder);
		}
	}

	CMuxer* COutputContext::CreateMuxer()
	{
		ThrowExceptionExpr(m_Context.StreamSize() == 0,
			"No stream in output context.\n");

		int nPrefix = StageToPrefix(EStage::ES_Mux, 0);

		CMuxer* Muxer = new CMuxer(*m_Cache, nPrefix);
		Muxer->Init(m_Context);

		m_vStages->emplace_back(Muxer);

		return Muxer;
	}

}
