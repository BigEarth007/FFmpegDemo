#include "pch.h"
#include "Editor.h"


namespace aveditor
{
	CEditor::CEditor()
		: m_OutputContext(*this, 0)
	{
		m_AVObject.SetEditor(this);

		m_vStages.emplace_back(new CDemuxer(this));
		m_vStages.emplace_back(new CDecoder(this, EStreamType::ST_Video));
		m_vStages.emplace_back(new CDecoder(this, EStreamType::ST_Audio));
		m_vStages.emplace_back(new CFiltrate(this));
		m_vStages.emplace_back(new CEncoder(this, EStreamType::ST_Video));
		m_vStages.emplace_back(new CEncoder(this, EStreamType::ST_Audio));
		m_vStages.emplace_back(new CMuxer(this));
	}

	CEditor::~CEditor()
	{
		Stop();
		Join();
		
		ReleaseVector(m_vStages);
	}

	FFormatContext& CEditor::OpenInputFile(
		const std::string& n_sFileName,
		const ETask n_eTask /*= ETask::T_Normal*/,
		const int& n_nStreams /*= kStreamAll*/,
		const AVInputFormat* n_InputFormat, 
		AVDictionary* n_Options)
	{
		int nContextIndex = (int)m_vInputContext.size();
		ThrowExceptionExpr(nContextIndex >= kEditorFactor / kEditorIndexFactor,
			"Too many input context: %d.\n", nContextIndex);

		if (!IsStop() && nContextIndex > 0)
		{
			for (size_t i = 0;i < m_vInputContext.size(); i++)
			{
				if (m_vInputContext[i]->GetContext().m_sName == n_sFileName)
				{
					return m_vInputContext[i]->GetContext();
				}
			}
		}

		CInputContext* InputContext = 
			new CInputContext(*this, nContextIndex);

		FFormatContext& Input = InputContext->OpenInputFile(
			n_sFileName, n_InputFormat, n_Options);

		// Set information of input context
		InputContext->SetTask(n_eTask);
		InputContext->MarkStream(n_nStreams);

		// Batch index
		int nBatchIndex = GetMaxBatchIndex();
		switch (n_eTask)
		{
		case ETask::T_Normal:
			nBatchIndex++;
			break;
		case ETask::T_AMixMain:
		case ETask::T_AMixBranch:
			{
				int nCount = 0;
				int nBatch = GetMaxAudioMixBatch(nCount);
				if (nBatch == -1) nBatchIndex++;
				else nBatchIndex = nBatch;

				InputContext->SetSubnumber(nCount);
			}
			break;
		}

		InputContext->SetBatchIndex(nBatchIndex);
		m_vInputContext.emplace_back(InputContext);

		return Input;
	}

	FFormatContext& CEditor::AllocOutputFile(const std::string& n_sFileName, 
		const AVOutputFormat* n_OutputFormat /*= nullptr*/, 
		const char* n_szFormatName /*= nullptr*/)
	{
		if (IsStop())
			m_OutputContext.AllocOutputFile(n_sFileName, n_OutputFormat, n_szFormatName);

		return m_OutputContext.GetContext();
	}

	void CEditor::OpenOutputFile()
	{
		m_OutputContext.OpenOutputFile();
	}

	void CEditor::CloseOutputFile()
	{
		m_OutputContext.CloseOutputFile();
	}

	CInputContext* CEditor::GetInputContext(const int& n_nContextIndex)
	{
		ThrowExceptionExpr(
			n_nContextIndex >= m_vInputContext.size() || n_nContextIndex < 0,
			"Invalid parameter.\n");

		return m_vInputContext[n_nContextIndex];
	}

	std::vector<int> CEditor::GetInputContextsByBatch(const int& n_nBatchIndex)
	{
		std::vector<int> vResult;

		for (int i = 0; i < (int)m_vInputContext.size(); i++)
		{
			if (m_vInputContext[i]->GetBatchIndex() == n_nBatchIndex)
				vResult.push_back(i);
		}

		return vResult;
	}

	const size_t CEditor::GetInputSize() const
	{
		return m_vInputContext.size();
	}

	COutputContext* CEditor::GetOutputContext()
	{
		return &m_OutputContext;
	}

	CAVObject* CEditor::GetAVObject()
	{
		return &m_AVObject;
	}

	void CEditor::SetOutputIOHandle(IAVIOHandle* n_Handle)
	{
		m_AVObject.SetOutputIOHandle(n_Handle);
	}

	void CEditor::SetInputContextHandle(IContextHandle* n_Handle, 
		const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		m_vInputContext[n_nContextIndex]->SetContextHandle(n_Handle);
	}

	void CEditor::SetInputIOHandle(IAVIOHandle* n_Handle, 
		const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		m_vInputContext[n_nContextIndex]->SetAVIOHandle(n_Handle);
	}

	void CEditor::SetMaxBufferSize(int n_nSize)
	{
		m_AVObject.SetMaxBufferSize(n_nSize);
	}

	const int CEditor::GetMaxBatchIndex() const
	{
		int nResult = -1;

		if (m_nMaxBatchIndex > -1) return m_nMaxBatchIndex;

		for (size_t i = 0; i < m_vInputContext.size(); i++)
		{
			if (m_vInputContext[i]->GetBatchIndex() > nResult)
				nResult = m_vInputContext[i]->GetBatchIndex();
		}

		return nResult;
	}

	const int CEditor::GetCurrentBatchIndex() const
	{
		return m_nCurBatchIndex;
	}

	const int CEditor::GetMaxAudioMixBatch(int& n_nCount) const
	{
		int nResult = -1;

		for (size_t i = 0; i < m_vInputContext.size(); i++)
		{
			if (m_vInputContext[i]->GetTask() != ETask::T_AMixMain &&
				m_vInputContext[i]->GetTask() != ETask::T_AMixBranch)
				continue;

			if (m_vInputContext[i]->GetBatchIndex() > nResult)
			{
				// a larger batch index, then count starts from 1;
				nResult = m_vInputContext[i]->GetBatchIndex();
				n_nCount = 1;
			}
			else if(m_vInputContext[i]->GetBatchIndex() == nResult)
				n_nCount++;
		}

		return nResult;
	}

	void CEditor::AddSelectedSection(const double n_dStart,
		const double n_dLength /*= 0*/, const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		m_vInputContext[n_nContextIndex]->AddSelectedSection(n_dStart, n_dLength);
	}

	void CEditor::RemoveSelectedSection(const size_t& n_nSectionIndex, 
		const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		m_vInputContext[n_nContextIndex]->RemoveSelectedSection(n_nSectionIndex);
	}

	void CEditor::WriteFrameDatas(EStreamType n_eStreamType,
		const void* n_Data, const int& n_nSize, const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		m_vInputContext[n_nContextIndex]->WriteFrameDatas(
			n_eStreamType, n_Data, n_nSize);
	}

	void CEditor::Start()
	{
		if (m_eStatus == EEditStatus::ES_Stopped)
		{
			Thread::Start();
		}
		else if (m_eStatus == EEditStatus::ES_Running)
		{
			Pause();
		}
		else if (m_eStatus == EEditStatus::ES_Pause)
		{
			SetStagesPause(false);
		}
	}

	void CEditor::Stop()
	{
		if (m_eStatus == EEditStatus::ES_Running)
		{
			m_eStatus = EEditStatus::ES_ForceStop;
			m_AVObject.SetEndFlag();
		}
		else if (m_eStatus == EEditStatus::ES_Stopped)
		{
			Release();
		}

		// If not call this function, it can't start again
		Thread::Stop();
	}

	void CEditor::Pause()
	{
		if (m_eStatus == EEditStatus::ES_Running)
			SetStagesPause(true);
	}

	bool CEditor::IsStop()
	{
		return Thread::IsStop() && m_eStatus == EEditStatus::ES_Stopped;
	}

	void CEditor::SetStagesPause(bool n_bPause)
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
		{
			m_vStages[i]->SetPause(n_bPause);
		}

		if (n_bPause && m_eStatus == EEditStatus::ES_Running)
			m_eStatus = EEditStatus::ES_Pause;
		else if (!n_bPause && m_eStatus == EEditStatus::ES_Pause)
			m_eStatus = EEditStatus::ES_Running;
	}

	EEditStatus CEditor::GetStatus() const
	{
		return m_eStatus;
	}

	FAudioFifo* CEditor::GetAudioFifo()
	{
		return m_AVObject.GetAudioFofo();
	}

	void CEditor::SetAudioPts(const int64_t n_nPts)
	{
		m_AVObject.SetAudioPts(n_nPts);
	}

	const int64_t CEditor::GetAudioPts() const
	{
		return m_AVObject.GetAudioPts();
	}

	void CEditor::Run()
	{
		try
		{
			CheckSelectedStreams();

			m_eStatus = EEditStatus::ES_Running;

			m_nMaxBatchIndex = GetMaxBatchIndex();
			m_nCurBatchIndex = 0;

			m_AVObject.Init();
			m_AVObject.StartBatch(m_nCurBatchIndex);
			SetStagesStart(true);

			while (true)
			{
				if (m_AVObject.IsBatchEnd())
				{
					if (m_nCurBatchIndex + 1 > m_nMaxBatchIndex ||
						m_eStatus == EEditStatus::ES_ForceStop)
						break;

					SetStagesPause(true);
					m_nCurBatchIndex++;
					m_AVObject.StartBatch(m_nCurBatchIndex);
					SetStagesPause(false);
				}

				Sleep(kSleepDelay * 50);
			}

			SetStagesStart(false);
			JoinStages();

			m_eStatus = EEditStatus::ES_Stopping;

			m_AVObject.Release();
		}
		catch (const std::exception& e)
		{
			AVDebug(e.what());
		}

		Release();
		m_eStatus = EEditStatus::ES_Stopped;

		Thread::Run();
	}

	void CEditor::CheckSelectedStreams()
	{
		int nMaxBatch = GetMaxBatchIndex();
		int nOutputStreams = m_OutputContext.StreamsCode();

		for (int i = 0; i <= nMaxBatch; i++)
		{
			int nCode = 0;
			auto v = GetInputContextsByBatch(i);
			for (size_t j = 0; j < v.size(); j++)
			{
				nCode |= GetInputContext(v[j])->StreamsCode();
			}

			ThrowExceptionExpr(nOutputStreams != nCode,
				"Selected streams for input/output context are not match\n");
		}
	}

	void CEditor::SetStagesStart(bool n_bStart)
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
		{
			if (n_bStart && m_vStages[i]->IsStop())
				m_vStages[i]->Start();
			else if (!n_bStart && !m_vStages[i]->IsStop())
				m_vStages[i]->Stop();
		}
	}

	void CEditor::JoinStages()
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
		{
			m_vStages[i]->Join();
		}
	}

	void CEditor::Release()
	{
		m_nMaxBatchIndex = -1;

		CloseOutputFile();
		m_OutputContext.Release();
		ReleaseVector(m_vInputContext);
	}

}
