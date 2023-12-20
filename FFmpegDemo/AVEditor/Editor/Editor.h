#pragma once


namespace aveditor
{
	class AVEDITOR_API CEditor : public Thread
	{
	public:
		CEditor();
		virtual ~CEditor();

		FFormatContext& OpenInputFile(const std::string& n_sFileName,
			const EJob n_eJob = EJob::EJ_Normal,
			const int& n_nStream = kStreamAll,
			const AVInputFormat* n_InputFormat = nullptr, 
			AVDictionary* n_Options = nullptr);

		FFormatContext& AllocOutputFile(const std::string& n_sFileName,
			const AVOutputFormat* n_OutputFormat = nullptr,
			const char* n_szFormatName = nullptr);

		void OpenOutputFile();
		void CloseOutputFile();

		FFormatContext& GetInputContext(const int& n_nContextIndex = 0);
		FFormatContext& GetOutputContext();
		FCache& GetCache();

		virtual void CreateDemuxer();

		virtual void CreateDecoder();

		virtual void CreateFilter(EStreamType n_eStreamType);

		virtual void CreateEncoder();

		// Equal CreateDecoder + CreateEncoder
		virtual void CreateTranscoder();

		virtual void CreateMuxer();

		virtual void Run();

		virtual CPlayer* CreatePlayer();

		void SetMaxCacheSize(int n_nIndex, unsigned int n_nMaxCacheSize);

		void CreateAllStage();
		void StartEdit();
		void StopEdit();
		void JoinEdit();
		virtual void Release();

	protected:
		FCache			m_Cache;

		std::vector<CInputContext*>	m_vInputContext;
		COutputContext				m_OutputContext;

		std::vector<IStage*>		m_vStages;
	};
}
