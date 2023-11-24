#pragma once


namespace aveditor
{
	struct AVEDITOR_API FAudioFifo
	{
		FAudioFifo() = default;
		~FAudioFifo();

		/*
		* Alloc audio fifo buffer
		* parameters:
		*	const AVSampleFormat& n_eSampleFormat:
		*		Sample format of the output frame
		*	const int& n_nChannels:
		*		Number of channels of output frame
		*	const int& n_nSamples:
		*		Number of samples per channel of output frame
		*/
		AVAudioFifo* Alloc(const AVSampleFormat& n_eSampleFormat,
			const int& n_nChannels, const int& n_nSamples);

		void Release();

		int Size();
		int OutputFrameSize();
		// The number of samples for next frame
		int NextSampleCount();

		// Push data into fifo buffer
		void Push(uint8_t** n_Samples, const int& n_nSize);

		// Pop frame out from fifo buffer
		void Pop(uint8_t** n_Samples, const int& n_nSize);
		void Pop(AVFrame* n_Frame);

		// Could read frame or not
		bool IsReadable();

		// Get parameters
		int					GetChannels()		{ return m_nChannels; }
		int					GetSamples()		{ return m_nSamples; }
		AVSampleFormat		GetSampleFormat()	{ return m_eSampleFormat; }

		AVAudioFifo*	m_Context = nullptr;

	protected:
		// Sample format of the output frame
		AVSampleFormat	m_eSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
		// Number of channels of output frame
		int				m_nChannels = 0;
		// Number of samples per channel of output frame
		int				m_nSamples = 0;
	};
}
