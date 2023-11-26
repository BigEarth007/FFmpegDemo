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
