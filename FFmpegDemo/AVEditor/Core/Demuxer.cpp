#include "pch.h"
#include "Demuxer.h"


namespace aveditor
{
	CDemuxer::CDemuxer(CEditor* n_Editor)
		: IStage(n_Editor)
	{
	}

	CDemuxer::~CDemuxer()
	{
		Release();
	}

	void CDemuxer::Release()
	{
		IStage::Release();
	}

	void CDemuxer::Run()
	{
		while (!IsStop())
		{
			PauseSleep();

			if (m_bPause) continue;

			if (m_AVObject)
			{
				if (AVERROR_EOF == m_AVObject->RunDemux())
					break;
			}
		}

		Thread::Run();
	}

}
