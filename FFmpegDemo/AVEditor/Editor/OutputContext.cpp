#include "pch.h"
#include "OutputContext.h"


namespace aveditor
{
	COutputContext::COutputContext(CEditor& n_Editor, const int n_nContextIndex)
		: CBaseContext(n_Editor, n_nContextIndex)
	{

	}

	COutputContext::~COutputContext()
	{
		CloseOutputFile();
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

	const int COutputContext::StreamsCode()
	{
		int ret = 0;

		auto v = m_Context.GetCodecContext();

		for (auto itr = v->begin(); itr != v->end(); itr++)
		{
			ret |= (1 << (int)itr->first);
		}

		return ret;
	}

}
