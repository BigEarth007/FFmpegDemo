#include "pch.h"
#include "Editor.h"


namespace aveditor
{
	CEditor::CEditor()
		: m_OutputContext(m_vStages, m_Cache, 0)
	{
	}

	CEditor::~CEditor()
	{
		Release();
	}

	FFormatContext& CEditor::OpenInputFile(
		const std::string& n_sFileName,
		const EJob n_eJob /*= EJob::EJ_Normal*/,
		const int& n_nStream /*= kStreamAll*/,
		const AVInputFormat* n_InputFormat, 
		AVDictionary* n_Options)
	{
		int nContextIndex = (int)m_vInputContext.size();
		ThrowExceptionExpr(nContextIndex >= kEditorFactor / kEditorIndexFactor,
			"Too many input context: %d.\n", nContextIndex);

		CInputContext* InputContext = 
			new CInputContext(m_vStages, m_Cache, nContextIndex);
		m_vInputContext.emplace_back(InputContext);

		FFormatContext& Input = InputContext->OpenInputFile(
			n_sFileName, n_InputFormat, n_Options);

		// Set input context object
		FContextInfo ContextInfo;
		ContextInfo.eJob = n_eJob;

		if (Input.m_Context)
		{
			// It's a valid input file
			AVStream* Stream = nullptr;

			for (unsigned int i = 0, j = 0; i < Input.m_Context->nb_streams; i++)
			{
				Stream = Input.m_Context->streams[i];
				if (Stream->codecpar->codec_type != AVMediaType::AVMEDIA_TYPE_UNKNOWN &&
					Stream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_NONE)
				{
					EStreamType eStreamType = kStreamIndex.at(
						Stream->codecpar->codec_type);
					if (n_nStream & (1 << (int)eStreamType))
					{
						// The stream index of context should start from 0, but the 
						// index of stream that we need may not start from 0. 
						// For example, detach audio stream from input context, 
						// and then write into output context
						ContextInfo.nStreams[(int)eStreamType] = j++;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < (int)EStreamType::EST_Max; i++)
			{
				if (n_nStream & (1 << i))
				{
					ContextInfo.nStreams[i] = i;
				}
			}
		}

		m_Cache.AddContent(ContextInfo);

		return Input;
	}

	FFormatContext& CEditor::AllocOutputFile(const std::string& n_sFileName, 
		const AVOutputFormat* n_OutputFormat /*= nullptr*/, 
		const char* n_szFormatName /*= nullptr*/)
	{
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

	FFormatContext& CEditor::GetInputContext(const int& n_nContextIndex)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		return m_vInputContext[n_nContextIndex]->GetContext();
	}

	FFormatContext& CEditor::GetOutputContext()
	{
		return m_OutputContext.GetContext();
	}

	FCache& CEditor::GetCache()
	{
		return m_Cache;
	}

	void CEditor::SetCallbackFillVideoFrame(
		std::function<void(AVFrame*, const void*, const int&)> n_func, 
		const int& n_nContextIndex)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
		"Invalid parameter.\n");

		CDemuxer* Demuxer = (CDemuxer*)m_Cache.GetContextInfo(n_nContextIndex)->Demuxer;
		if (Demuxer)
		{
			Demuxer->SetCallbackFillVideoFrame(n_func);
		}
	}

	void CEditor::SetCallbackFillAudioFrame(
		std::function<void(AVFrame*, const void*, const int&)> n_func, 
		const int& n_nContextIndex)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
		"Invalid parameter.\n");

		CDemuxer* Demuxer = (CDemuxer*)m_Cache.GetContextInfo(n_nContextIndex)->Demuxer;
		if (Demuxer)
		{
			Demuxer->SetCallbackFillAudioFrame(n_func);
		}
	}

	void CEditor::WriteFrameDatas(EStreamType n_eStreamType, 
		const void* n_Data, const int& n_nSize, const int& n_nContextIndex /*= 0*/)
	{
		ThrowExceptionExpr(n_nContextIndex >= m_vInputContext.size(),
			"Invalid parameter.\n");

		CDemuxer* Demuxer = (CDemuxer*)m_Cache.GetContextInfo(n_nContextIndex)->Demuxer;
		if (Demuxer)
		{
			Demuxer->WriteFrameDatas(n_eStreamType, n_Data, n_nSize);
		}
	}

	void CEditor::CreateDemuxer()
	{
		std::vector<FContextInfo>& vContextInfos = m_Cache.GetContextInfos();
		for (size_t i = 0; i < m_vInputContext.size(); i++)
		{
			CDemuxer* Muxer = m_vInputContext[i]->CreateDemuxer(m_OutputContext.GetContext());
			vContextInfos[i].Demuxer = Muxer;
		}
	}

	void CEditor::CreateDecoder()
	{
		for (size_t i = 0; i < m_vInputContext.size(); i++)
			m_vInputContext[i]->CreateDecoder(m_OutputContext.GetContext());
	}

	void CEditor::CreateFilter(EStreamType n_eStreamType)
	{
		int nPrefix = 0;
		int nFilterIndex = 0;
		std::vector<FContextInfo>& vContextInfos = m_Cache.GetContextInfos();

		for (size_t i = 0; i < vContextInfos.size(); i++)
		{
			if (vContextInfos[i].eJob == EJob::EJ_AMixMain)
			{
				nPrefix = m_Cache.CreateCache(EStage::ES_Filter,
					EItemType::EIT_Frame, (int)i, n_eStreamType);

				vContextInfos[i].nFilterIndex = nFilterIndex++;
			}
			else if (vContextInfos[i].eJob == EJob::EJ_AMixBranch)
				vContextInfos[i].nFilterIndex = nFilterIndex++;
		}

		if (nPrefix == 0) return;

		std::map<EStreamType, FCodecContext>* mOutputCodecContext =
			m_OutputContext.GetContext().GetCodecContext();

		auto itr = mOutputCodecContext->find(n_eStreamType);
		if (itr == mOutputCodecContext->end()) return;

		CFiltrate* Filtrate = new CFiltrate(m_Cache, nPrefix, n_eStreamType);
		Filtrate->Init(itr->second);

		m_vStages.emplace_back(Filtrate);
	}

	void CEditor::CreateEncoder()
	{
		for (size_t i = 0; i < m_vInputContext.size(); i++)
			m_vInputContext[i]->CreateEncoder(m_OutputContext.GetContext());
	}

	void CEditor::CreateTranscoder()
	{
		for (size_t i = 0; i < m_vInputContext.size(); i++)
			m_vInputContext[i]->CreateTranscoder(m_OutputContext.GetContext());
	}

	void CEditor::CreateMuxer()
	{
		CMuxer* Muxer = m_OutputContext.CreateMuxer();
		Muxer->SetFinishedCallback([this, &Muxer]() {
			StopEdit();
			CloseOutputFile();
			}
		);
	}

	void CEditor::Run()
	{
		StartEdit();
		JoinEdit();
	}

	IPlayer* CEditor::CreatePlayer(IPlayer* n_Player)
	{
		ThrowExceptionExpr(m_vInputContext.size() == 0, "No input context.\n");

		CreateDemuxer();
		CreateDecoder();

		IPlayer* Player = m_vInputContext[0]->CreatePlayer(n_Player);

		Player->SetFinishedCallback([this]() {
			StopEdit();
			m_Cache.Release();
			}
		);

		return Player;
	}

	void CEditor::SetMaxCacheSize(int n_nIndex, unsigned int n_nMaxCacheSize)
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
		{
			if (n_nIndex == -1 || i == n_nIndex)
				m_vStages[i]->SetMaxCacheSize(n_nMaxCacheSize); 
		}
	}

	void CEditor::CreateAllStage()
	{
		CreateDemuxer();

		if (GetOutputContext().m_Context)
		{
			bool bFilter = false;
			std::vector<FContextInfo>& vContextInfos = m_Cache.GetContextInfos();

			for (size_t i = 0; i < vContextInfos.size(); i++)
			{
				if (vContextInfos[i].eJob == EJob::EJ_AMixMain ||
					vContextInfos[i].eJob == EJob::EJ_AMixBranch)
				{
					bFilter = true;
					break;
				}
			}

			if (bFilter)
			{
				CreateDecoder();
				CreateFilter(EStreamType::EST_Audio);
				CreateEncoder();
			}
			else
				CreateTranscoder();

			CreateMuxer();
		}
	}

	void CEditor::StartEdit()
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
			m_vStages[i]->Start();
	}

	void CEditor::StopEdit()
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
			m_vStages[i]->Stop();
	}

	void CEditor::JoinEdit()
	{
		for (size_t i = 0; i < m_vStages.size(); i++)
			m_vStages[i]->Join();
	}

	void CEditor::Release()
	{
		StopEdit();
		JoinEdit();

		ReleaseVector(m_vStages);
		m_OutputContext.Release();
		ReleaseVector(m_vInputContext);

		m_Cache.Release();
	}

}
