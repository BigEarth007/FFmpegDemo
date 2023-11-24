#include "pch.h"
#include "BaseContext.h"


namespace aveditor
{
	CBaseContext::CBaseContext(std::vector<IStage*>& n_vStages, 
		FCache& n_Cache, const int n_nContextIndex)
	{
		m_vStages = &n_vStages;
		m_Cache = &n_Cache;
		m_nContextIndex = n_nContextIndex;
	}

	CBaseContext::~CBaseContext()
	{
		Release();
	}

	FFormatContext& CBaseContext::GetContext()
	{
		return m_Context;
	}

	void CBaseContext::Release()
	{
		m_Context.Release();
	}

}
