#include "pch.h"
#include "BaseContext.h"


namespace aveditor
{
	CBaseContext::CBaseContext(CEditor& n_Editor, const int n_nContextIndex)
	{
		m_nContextIndex = n_nContextIndex;

		m_Editor = &n_Editor;
	}

	CBaseContext::~CBaseContext()
	{
		Release();
	}

	FFormatContext& CBaseContext::GetContext()
	{
		return m_Context;
	}

	const bool CBaseContext::IsValid() const
	{
		return m_Context.IsValid();
	}

	const int CBaseContext::GetContextIndex() const
	{
		return m_nContextIndex;
	}

	void CBaseContext::Release()
	{
		m_Context.Release();
	}

	void CBaseContext::SetContextHandle(IContextHandle* n_CtxHandle)
	{
		m_CtxHandle = n_CtxHandle;

		if (m_CtxHandle && m_Context.IsValid())
		{
			auto vCodec = m_Context.GetCodecContext(EStreamType::ST_Video);
			if (vCodec)
			{
				int nPlanes = GetPixFmtPlaneCount(vCodec->pix_fmt);
				m_CtxHandle->SetVideoPlanes(nPlanes);
			}

			auto aCodec = m_Context.GetCodecContext(EStreamType::ST_Audio);
			if (aCodec)
			{
				int nIsPlanar = IsSampleFmtPlanar(aCodec->sample_fmt);
				int nBytesPerSample = GetBytesPerSample(aCodec->sample_fmt);

				m_CtxHandle->SetAudioPlanar(nIsPlanar);
				m_CtxHandle->SetBytesPerSample(nBytesPerSample);
				m_CtxHandle->SetChannelCount(aCodec->ch_layout.nb_channels);
			}
		}
	}

	void CBaseContext::SetAVIOHandle(IAVIOHandle* n_AVIOHandle)
	{
		if (m_CtxHandle) m_CtxHandle->SetAVIOHandle(n_AVIOHandle);
	}

}
