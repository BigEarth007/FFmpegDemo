#include "pch.h"
#include "AVObject.h"


namespace aveditor
{
	CAVObject::CAVObject()
	{

	}

	CAVObject::~CAVObject()
	{
		Release();
	}

	void CAVObject::Init()
	{
		ThrowExceptionExpr(!m_Editor, "Call function SetEditor please!\n");

		m_EncodeComp.SetEditor(m_Editor);
		m_EncodeComp.Init();

		m_MuxerComp.SetEditor(m_Editor);
		m_MuxerComp.Init();

		m_AudioMixComp.SetEditor(m_Editor);
	}

	void CAVObject::Release()
	{
		ResetComponents();
	}

	int CAVObject::RunDemux()
	{
		int ret = AVERROR_EOF;

		for (size_t i = 0; i < m_vDemuxComp.size(); i++)
		{
			if (AVERROR_EOF != m_vDemuxComp[i]->Run())
				ret = 0;
		}

		return ret;
	}

	int CAVObject::RunDecode(EStreamType n_eStreamType)
	{
		int ret = AVERROR_EOF;

		for (size_t i = 0; i < m_vDecodeComp.size(); i++)
		{
			if (AVERROR_EOF != m_vDecodeComp[i]->Run(n_eStreamType))
				ret = 0;
		}

		return ret;
	}

	int CAVObject::RunFilter()
	{
		return m_AudioMixComp.Run();
	}

	int CAVObject::RunEncode(EStreamType n_eStreamType)
	{
		return m_EncodeComp.Run(n_eStreamType);
	}
	
	int CAVObject::RunMuxer()
	{
		return m_MuxerComp.Run();
	}

	void CAVObject::SetEditor(CEditor* n_Editor)
	{
		m_Editor = n_Editor;
	}

	const CEditor* CAVObject::GetEditor() const
	{
		return m_Editor;
	}

	void CAVObject::StartBatch(int n_nBatchIndex)
	{
		ResetComponents();

		if (n_nBatchIndex == 0) m_MuxerComp.SetDuration(0);

		auto vInputs = m_Editor->GetInputContextsByBatch(n_nBatchIndex);

		for (size_t i = 0; i < vInputs.size(); i++)
		{
			CDemuxComponent* InputComp = new CDemuxComponent();
			InputComp->SetEditor(m_Editor);
			InputComp->Init(vInputs[i]);
			InputComp->SetMaxBufferSize(m_nMaxBufferSize);
			m_vDemuxComp.emplace_back(InputComp);

			CDecodeComponent* DecodeComp = new CDecodeComponent();
			DecodeComp->SetEditor(m_Editor);
			DecodeComp->Init(vInputs[i]);
			DecodeComp->SetMaxBufferSize(m_nMaxBufferSize);
			m_vDecodeComp.emplace_back(DecodeComp);

			m_Editor->GetInputContext(vInputs[i])->SetAVIOHandle(InputComp);

			InputComp->SetIOHandle(DecodeComp);
			DecodeComp->SetIOHandle(&m_AudioMixComp);
		}

		m_AudioMixComp.Init(n_nBatchIndex);

		m_AudioMixComp.SetIOHandle(&m_EncodeComp);
		m_EncodeComp.SetIOHandle(&m_MuxerComp);

		m_AudioMixComp.SetMaxBufferSize(m_nMaxBufferSize);
		m_EncodeComp.SetMaxBufferSize(m_nMaxBufferSize);
		m_MuxerComp.SetMaxBufferSize(m_nMaxBufferSize);
	}

	const bool CAVObject::IsBatchEnd() const
	{
		return m_MuxerComp.GetEndFlag();
	}

	void CAVObject::SetEndFlag()
	{
		for (size_t i = 0; i < m_vDecodeComp.size(); i++)
		{
			m_vDecodeComp[i]->ForceStop();
		}

		m_AudioMixComp.ForceStop();
		m_EncodeComp.ForceStop();
		m_MuxerComp.ForceStop();

		for (size_t i = 0; i < m_vDemuxComp.size(); i++)
		{
			m_vDemuxComp[i]->SetEndFlag(true);
			m_vDemuxComp[i]->WriteEndData();
		}
	}

	void CAVObject::SetMaxBufferSize(int n_nSize)
	{
		m_nMaxBufferSize = n_nSize;
	}

	void CAVObject::SetOutputIOHandle(IAVIOHandle* n_Handle)
	{
		m_MuxerComp.SetIOHandle(n_Handle);
	}

	void CAVObject::ResetComponents()
	{
		double dDuration = Duration();
		// Set current duration for media appending
		m_MuxerComp.AddDuration(dDuration);

		ReleaseVector(m_vDemuxComp);
		ReleaseVector(m_vDecodeComp);
		m_AudioMixComp.Release();
		m_EncodeComp.Release();
		m_MuxerComp.Release();
	}

	double CAVObject::Duration()
	{
		double dDuration = 0;

		for (size_t i = 0; i < m_vDemuxComp.size(); i++)
		{
			CInputContext* Input = m_vDemuxComp[i]->GetInputContext();
			if (Input->GetTask() == ETask::T_Normal)
				dDuration = Input->Duration();
			else
			{
				if (Input->GetStreamIndex(EStreamType::ST_Video) > -1)
					dDuration = Input->Duration();
				else
				{
					if (Input->Duration() > dDuration)
						dDuration = Input->Duration();
				}
			}
		}

		return dDuration;
	}

}