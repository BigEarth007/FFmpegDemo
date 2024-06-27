#pragma once


namespace aveditor
{
	enum class EEditStatus
	{
		// Force to stop
		ES_ForceStop,
		// It's stopped
		ES_Stopped,
		// It's paused
		ES_Pause,
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
			const ETask n_eTask = ETask::T_Normal,
			const int& n_nStreams = kStreamAll,
			const AVInputFormat* n_InputFormat = nullptr, 
			AVDictionary* n_Options = nullptr);

		// Create an output file
		FFormatContext& AllocOutputFile(const std::string& n_sFileName,
			const AVOutputFormat* n_OutputFormat = nullptr,
			const char* n_szFormatName = nullptr);

		void OpenOutputFile();
		void CloseOutputFile();

		// Get input context
		CInputContext* GetInputContext(const int& n_nContextIndex = 0);
		// Get input contexts by batch index
		std::vector<int> GetInputContextsByBatch(const int& n_nBatchIndex);
		// Get number of input context
		const size_t GetInputSize() const;

		// Get output context
		COutputContext* GetOutputContext();

		// Get AVObject
		CAVObject* GetAVObject();

		// Set Audio/Video IO handle for output 
		void SetOutputIOHandle(IAVIOHandle* n_Handle);

		// Set Audio/Video IO handle for input 
		void SetInputContextHandle(IContextHandle* n_Handle,
			const int& n_nContextIndex = 0);

		// Set Audio/Video IO handle for input 
		void SetInputIOHandle(IAVIOHandle* n_Handle, 
			const int& n_nContextIndex = 0);

		// Set max buffer queue size
		void SetMaxBufferSize(int n_nSize);

		// Get max batch index of all input contexts
		const int GetMaxBatchIndex() const;

		// Get current batch index of all input contexts
		const int GetCurrentBatchIndex() const;

		// Get max batch index of all input contexts that for audio mix
		const int GetMaxAudioMixBatch(int& n_nCount) const;

		// Set time section; it's second;
		// const double n_dStart: start timestamp
		// const double n_dLength: time duration, 0 means to the end
		void AddSelectedSection(const double n_dStart, const double n_dLength = 0,
			const int& n_nContextIndex = 0);

		// Remove section
		void RemoveSelectedSection(const size_t& n_nSectionIndex,
			const int& n_nContextIndex = 0);

		// Write Frame data into this empty input file
		// if n_Data is nullptr, then write nullptr frame
		void WriteFrameDatas(EStreamType n_eStreamType,
			const void* n_Data, const int& n_nSize, 
			const int& n_nContextIndex = 0);

		virtual void Start();
		virtual void Stop();
		virtual void Pause();
		virtual bool IsStop();

		void SetStagesPause(bool n_bPause);

		EEditStatus GetStatus() const;

		FAudioFifo* GetAudioFifo();

		void SetAudioPts(const int64_t n_nPts);
		const int64_t GetAudioPts() const;

	protected:
		virtual void Run();

		// Selected streams should be equal between input and output
		void CheckSelectedStreams();

		void SetStagesStart(bool n_bStart);
		void JoinStages();

	protected:
		virtual void Release();

	protected:
		std::vector<IStage*>		m_vStages;
		std::vector<CInputContext*>	m_vInputContext;
		COutputContext				m_OutputContext;

		CAVObject					m_AVObject;

		EEditStatus					m_eStatus = EEditStatus::ES_Stopped;

		int							m_nMaxBatchIndex = -1;
		int							m_nCurBatchIndex = 0;
	};
}
