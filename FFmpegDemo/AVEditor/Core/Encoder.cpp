#include "pch.h"
#include "Encoder.h"


namespace aveditor
{
	CEncoder::CEncoder(CEditor* n_Editor, EStreamType n_eStreamType)
		: IStage(n_Editor)
	{
		m_eStreamType = n_eStreamType;
	}

	CEncoder::~CEncoder()
	{
		Release();
	}

	void CEncoder::Run()
	{
		while (!IsStop())
		{
			PauseSleep();

			if (m_bPause) continue;

			if (m_AVObject)
			{
				if (AVERROR_EOF == m_AVObject->RunEncode(m_eStreamType))
					break;
			}
		}

		Thread::Run();
	}

	void CEncoder::Release()
	{
		IStage::Release();
	}
}
