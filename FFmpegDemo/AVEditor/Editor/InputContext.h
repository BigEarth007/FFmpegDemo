#pragma once


namespace aveditor
{
	class AVEDITOR_API CInputContext : public CBaseContext
	{
	public:
		CInputContext(CEditor& n_Editor, const int n_nContextIndex);
		virtual ~CInputContext();

		FFormatContext& OpenInputFile(const std::string& n_sFileName,
			const AVInputFormat* n_InputFormat = nullptr,
			AVDictionary* n_Options = nullptr);

		// The length of the context
		const double Duration();

		const double GetSectionFrom() const;
		const double GetSectionTo() const;

		// Get context name
		const std::string GetName() const;

		// Set what to do with this context
		void SetTask(const ETask n_eTask);
		// Get what to do with this context
		const ETask GetTask() const;

		// Mark stream of context as task
		void MarkStream(const int n_nStreams);
		void MarkStream(EStreamType n_eStreamType, const int n_nStreamIndex);
		// Unmark stream of context as task
		void UnmarkStream(EStreamType n_eStreamType);
		// Get stream index of stream
		const int GetStreamIndex(EStreamType n_eStreamType) const;
		// Combine streams mark, which indicates selected streams
		const int StreamsCode() const;

		// Set subnumber
		void SetSubnumber(const int n_nSubnumber);
		// Get subnumber
		const int GetSubnumber() const;

		// Set Batch index
		void SetBatchIndex(const int n_nBatchIndex);
		// Get Batch index
		const int GetBatchIndex() const;

		// Set time section; it's second;
		// const double n_dStart: start timestamp
		// const double n_dDuration: time duration, 0 means to the end
		void SelectSection(const double n_dStart, const double n_dDuration = 0);

		// Write Frame data into this empty input file
		// if n_Data is nullptr, then write nullptr frame
		void WriteFrameDatas(EStreamType n_eStreamType,
			const void* n_Data, const int& n_nSize);

		virtual void Release();

	protected:
		AVFrame* WriteVideoFrame(AVCodecContext* n_CodecContext,
			const void* n_Data, const int& n_nSize);
		AVFrame* WriteAudioFrame(AVCodecContext* n_CodecContext,
			const void* n_Data, const int& n_nSize);

	protected:
		// What to do with this input context
		ETask	m_eTask = ETask::T_Normal;

		// The index of streams
		// -1: ignore the corresponding  stream in input context
		// >=0: if write this stream into output context, it's the real stream index
		int		m_nStreams[(int)EStreamType::ST_Size] = { -1, -1, -1 };

		// The order to do with input context
		// It may not equal to context index
		int		m_nBatchIndex = 0;

		// Index of the batch
		int		m_nSubnumber = 0;

		// Section start time; 0: meams not in use
		double	m_dSectionFrom = 0;
		// Section end time; 0: meams to end
		double	m_dSectionTo = 0;

		// For recording PCM data begin
		// Frame index of video
		int64_t	m_nVideoFrameIndex = 0;
		// PTS of audio frame
		int64_t	m_nAudioFramePts = 0;
	};
}
