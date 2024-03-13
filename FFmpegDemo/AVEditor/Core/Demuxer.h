#pragma once


namespace aveditor
{
	class AVEDITOR_API CDemuxer : public IStage
	{
	public:
		CDemuxer(FCache& n_Cache, const int& n_nPrefix);
		~CDemuxer();

		// Set the input file context
		void Init(FFormatContext& n_InputContext, 
			FFormatContext* n_OutputContext = nullptr);
		// Release
		void Release();

		// Set time section; it's second;
		// const double n_dStart: start timestamp
		// const double n_dDuration: time duration, 0 means to the end
		void SelectSection(const double n_dStart, const double n_dDuration = 0);

		// Write Frame data into this empty input file
		// if n_Data is nullptr, then write the last frame
		void WriteVideoFrame(const uint8_t** n_Data = nullptr, 
			const int* n_LineSize = nullptr, const int& n_nRows = 0);

		// Write Frame data into this empty input file
		// if n_Data is nullptr, then write the last frame
		void WriteAudioFrame(const uint8_t** n_Data = nullptr,
			const int* n_LineSize = nullptr, const int& n_nRows = 0);

		virtual void Run();

	protected:
		/* Set the duration of this input context */
		void SetDuration();

	protected:
		// Input file
		FFormatContext* m_InputContext = nullptr;
		// Output file
		FFormatContext* m_OutputContext = nullptr;

		// Section start time; 0: meams not in use
		double			m_dSectionFrom = 0;
		// Section end time; 0: meams to end
		double			m_dSectionTo = 0;

		// Frame index of video
		int64_t			m_nVideoFrameIndex = 0;
		// Pts of audio frame
		int64_t			m_nAudioFramePts = 0;
	};
}
