#include "pch.h"
#include "Decoder.h"


namespace aveditor
{
	CDecoder::CDecoder(CEditor* n_Editor, EStreamType n_eStreamType)
		: IStage(n_Editor)
	{
		m_eStreamType = n_eStreamType;
	}

	CDecoder::~CDecoder()
	{
		Release();
	}

	void CDecoder::Run()
	{
		while (!IsStop())
		{
			PauseSleep();

			if (m_bPause) continue;

			if (m_AVObject)
			{
				if (AVERROR_EOF == m_AVObject->RunDecode(m_eStreamType))
					break;
			}
		}

		Thread::Run();
	}

	void CDecoder::Release()
	{
		IStage::Release();

	}
}