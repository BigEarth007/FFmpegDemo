#pragma once


namespace aveditor
{
	class AVEDITOR_API CConvert
	{
	public:
		CConvert();
		~CConvert();

		void Init(FCodecContext& n_InputCodecContext,
			FCodecContext& n_OutputCodecContext);

		/*
		* Do with n_Frame
		* return value
		*	0: nothing to do
		*	1: convert the n_Frame
		*/
		int Process(AVFrame* n_Frame, const int& n_nKey);

		// If all AVFrame are read from the queue, we should pop the last samples 
		// from AVAudioFifo buffer
		void CleanAudioFifo(const int& n_nKey);

		// Set this callback to do with the AVFrame that have been converted
		void SetFinishedCallback(std::function<int(AVFrame*, const int&)> n_func);

		void Release();

	protected:
		// Crete converter for decoded frame
		void CreateConverter(FCodecContext& n_InputCodecContext,
			FCodecContext& n_OutputCodecContext);
		// Create Audio Fifo buffer
		void CreateAudioFifo(FCodecContext& n_InputCodecContext,
			FCodecContext& n_OutputCodecContext);

		// Convert the video frame
		void VideoFrame(AVFrame* n_Frame, const int& n_nKey);
		// Convert the audio frame
		void AudioFrame(AVFrame* n_Frame, const int& n_nKey);

		// Pop frame from AVAudioFifo buffer
		void PopFromFifo(const int& n_nKey);

		// The last step to convert AVFrame
		void FinishedConvert(AVFrame* n_Frame, const int& n_nKey);

	protected:
		std::function<int(AVFrame*, const int&)> m_funcFinished = nullptr;

		// Converter
		FCale			m_Cale;
		FResample		m_Resample;

		// Audio FIFO buffer
		FAudioFifo		m_AudioFifo;
		// The counter of Audio AVFrame samples
		int64_t			m_nAudioFramePts = 0;
		// Output codec context
		AVCodecContext* m_OutputCodecContext = nullptr;
	};
}
