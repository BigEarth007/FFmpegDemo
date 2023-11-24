#pragma once


namespace aveditor
{
	struct AVEDITOR_API FResample
	{
		FResample() = default;
		~FResample();

		// Alloc swr context
		SwrContext* Alloc(
			const AVChannelLayout* n_InputChannelLayout,
			const AVSampleFormat& n_eInputSampleFormat,
			const int& n_nInputSampleRate,
			const AVChannelLayout* n_OutputChannelLayout,
			const AVSampleFormat& n_eOutputSampleFormat,
			const int& n_nOutputSampleRate);

		SwrContext* Alloc(AVCodecContext* n_InputCodecContext,
			AVCodecContext* n_OutputCodecContext);

		// Release 
		void Release();

		// Cover audio data
		// return number of samples output per channel, negative value on error
		int Cover(const uint8_t** n_InputData, int n_nInputFrameSize);
		int Cover(const uint8_t** n_InputData, int n_nInputFrameSize, 
			uint8_t** n_OutputData, int n_nOutputFrameSize);
		int Cover(const AVFrame* n_InputFrame);

		void AllocCovertedSample(const int& n_nChannels, const int& n_nFrameSize,
			const AVSampleFormat& n_eOutputSampleFormat);

		// Get parameters
		int					GetInputChannels()		{ return m_InputChannels; }
		int					GetInputSampleRate()	{ return m_nInputSampleRate; }
		AVSampleFormat		GetInputSampleFormat()	{ return m_eInputSampleFormat; }
		int					GetOutputChannels()		{ return m_OutputChannels; }
		int					GetOutputSampleRate()	{ return m_nOutputSampleRate; }
		AVSampleFormat		GetOutputSampleFormat()	{ return m_eOutputSampleFormat; }

		SwrContext*			m_Context = nullptr;

		// Output data when converter audio frame data
		uint8_t**			m_CoverData = nullptr;
		int					m_nSamples = 0;

	protected:
		int					m_InputChannels			= 0;
		int					m_nInputSampleRate		= 0;
		AVSampleFormat		m_eInputSampleFormat	= AVSampleFormat::AV_SAMPLE_FMT_NONE;
		int					m_OutputChannels		= 0;
		int					m_nOutputSampleRate		= 0;
		AVSampleFormat		m_eOutputSampleFormat	= AVSampleFormat::AV_SAMPLE_FMT_NONE;
	};
}
