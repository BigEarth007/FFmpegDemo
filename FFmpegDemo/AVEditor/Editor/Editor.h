#pragma once


namespace aveditor
{
	enum class EEditStatus
	{
		// It's stopped
		ES_Stopped,
		// It's running
		ES_Running,
		// It's stopping
		ES_Stopping,
	};

	class AVEDITOR_API CEditor : public Thread
	{
	public:
		CEditor();
		virtual ~CEditor();

		// Add an input file, if n_sFileName == "", then add an empty input file
		// For empty input file, it can record PCM data, and writes into output file
		FFormatContext& OpenInputFile(const std::string& n_sFileName,
			const EJob n_eJob = EJob::EJ_Normal,
			const int& n_nStream = kStreamAll,
			const AVInputFormat* n_InputFormat = nullptr, 
			AVDictionary* n_Options = nullptr);

		// Create an output file
		FFormatContext& AllocOutputFile(const std::string& n_sFileName,
			const AVOutputFormat* n_OutputFormat = nullptr,
			const char* n_szFormatName = nullptr);

		void OpenOutputFile();
		void CloseOutputFile();

		FFormatContext& GetInputContext(const int& n_nContextIndex = 0);
		FFormatContext& GetOutputContext();
		FCache& GetCache();

		// Set the callback function to fill data buffer of video AVFrame
		void SetCallbackFillVideoFrame(
			std::function<void(AVFrame*, const void*, const int&)> n_func, 
			const int& n_nContextIndex = 0);
		// Set the callback function to fill data buffer of audio AVFrame
		void SetCallbackFillAudioFrame(
			std::function<void(AVFrame*, const void*, const int&)> n_func, 
			const int& n_nContextIndex = 0);

		// Write Frame data into this empty input file
		// if n_Data is nullptr, then write nullptr frame
		void WriteFrameDatas(EStreamType n_eStreamType,
			const void* n_Data, const int& n_nSize, 
			const int& n_nContextIndex = 0);

		virtual void CreateDemuxer();

		virtual void CreateDecoder();

		virtual void CreateFilter(EStreamType n_eStreamType);

		virtual void CreateEncoder();

		// Equal CreateDecoder + CreateEncoder
		virtual void CreateTranscoder();

		virtual void CreateMuxer();

		virtual void Start();
		virtual void Run();
		virtual void Stop();
		virtual bool IsStop();

		virtual IPlayer* CreatePlayer(IPlayer* n_Player = nullptr);

		void SetMaxCacheSize(int n_nIndex, unsigned int n_nMaxCacheSize);

		void CreateAllStage();

		EEditStatus GetStstua();

	protected:
		void StartEdit();
		void StopEdit();
		bool IsAllStopped();
		void JoinEdit();
		virtual void Release();

	protected:
		FCache			m_Cache;

		std::vector<CInputContext*>	m_vInputContext;
		COutputContext				m_OutputContext;

		std::vector<IStage*>		m_vStages;

		EEditStatus					m_eStatus = EEditStatus::ES_Stopped;
	};
}
