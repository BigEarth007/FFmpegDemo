#include "pch.h"
#include "Stage.h"


namespace aveditor
{
	IStage::IStage(FCache& n_Cache, const int& n_nPrefix,
		const EStreamType& n_eStreamType)
	{
		BaseInit(n_Cache, n_nPrefix, n_eStreamType);
	}

	IStage::~IStage()
	{

	}

	void IStage::BaseInit(FCache& n_Cache, const int& n_nPrefix, 
		const EStreamType& n_eStreamType)
	{
		m_Cache = &n_Cache;
		m_nCurrentPrefix = n_nPrefix;
		m_eStreamType = n_eStreamType;

		m_nPreviousPrefix = m_Cache->GetPreviousKeyPrefix(
			m_nCurrentPrefix + (int)m_eStreamType);
	}

	int IStage::GetContextIndex()
	{
		return m_nCurrentPrefix % kEditorFactor / kEditorIndexFactor;
	}

	void IStage::SetMaxCacheSize(unsigned int n_nMaxCacheSize)
	{
		m_nMaxCacheSize = n_nMaxCacheSize;
	}

	void IStage::ConsumeCache(const int& n_nKey)
	{
		if (m_nMaxCacheSize > 0 && m_nMaxCacheSize < m_Cache->Size(n_nKey))
		{
			int nMin = m_nMaxCacheSize / 2;
			while (!IsStop() && m_Cache->Size(n_nKey) > nMin)
			{
				Sleep(kSleepDelay * 10);
			}
		}
	}

	void IStage::StageSleep()
	{
		//Sleep(kSleepDelay);
	}

}