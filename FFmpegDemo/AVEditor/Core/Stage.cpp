#include "pch.h"
#include "Stage.h"


namespace aveditor
{
	IStage::IStage(CEditor* n_Editor)
	{
		BaseInit(n_Editor);
	}

	IStage::~IStage()
	{
	}

	void IStage::BaseInit(CEditor* n_Editor)
	{
		m_Editor = n_Editor;
		m_AVObject = m_Editor->GetAVObject();
	}

	CAVObject* IStage::GetAVObject() const
	{
		return m_AVObject;
	}

	void IStage::SetPause(bool n_bPause)
	{
		m_bPause = n_bPause;
	}

	void IStage::StageSleep()
	{
		//Sleep(kSleepDelay);
	}

	void IStage::PauseSleep(int n_nMillisecond /*= kSleepDelay * 20*/)
	{
		if (m_bPause) Sleep(n_nMillisecond);
	}

	void IStage::Release()
	{
		Stop();
		Join();
	}

}