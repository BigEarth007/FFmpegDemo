#include "pch.h"
#include "Muxer.h"


namespace aveditor
{
	CMuxer::CMuxer(CEditor* n_Editor)
		: IStage(n_Editor)
	{
	}

	CMuxer::~CMuxer()
	{
	}

	void CMuxer::Run()
	{
		while (!IsStop())
		{
			PauseSleep();

			if (m_bPause) continue;

			if (m_AVObject)
			{
				if (AVERROR_EOF == m_AVObject->RunMuxer())
					break;
			}
		}

		Thread::Run();
	}

}
