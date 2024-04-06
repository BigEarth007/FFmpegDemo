#include "pch.h"
#include "Filtrate.h"


namespace aveditor
{
	CFiltrate::CFiltrate(CEditor* n_Editor)
		: IStage(n_Editor)
	{
	}

	CFiltrate::~CFiltrate()
	{
		Release();
	}

	void CFiltrate::Run()
	{
		while (!IsStop())
		{
			PauseSleep();

			if (m_bPause) continue;

			if (m_AVObject)
			{
				if (AVERROR_EOF == m_AVObject->RunFilter())
					break;
			}
		}

		Thread::Run();
	}

	void CFiltrate::Release()
	{
		IStage::Release();
	}
}
