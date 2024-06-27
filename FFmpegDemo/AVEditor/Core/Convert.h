	#pragma once


namespace aveditor
{
	class AVEDITOR_API IFrameConvert
	{
	public:
		IFrameConvert();
		virtual ~IFrameConvert();

		virtual void Init(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext);

		/*
		* Do with n_Frame
		* return value
		*	0: nothing to do
		*	1: convert the n_Frame
		*/
		virtual int Process(AVFrame* n_Frame);

		virtual void SetAudioFifo(FAudioFifo* n_AudioFifo);
		virtual void CleanCache();

		// Is the converter valid
		virtual const bool IsValid() const;

		// Set this callback to do with the AVFrame that have been converted
		void SetFinishedCallback(std::function<int(AVFrame*)> n_func);

		void SetConvertPts(const int64_t n_nPts);
		const int64_t GetConvertPts() const;

		virtual void Release();

	protected:
		// The last step to convert AVFrame
		void FinishedConvert(AVFrame* n_Frame);

	protected:
		std::function<int(AVFrame*)> m_funcFinished = nullptr;

		// Output codec context
		AVCodecContext* m_OutputCodecContext = nullptr;

		int64_t			m_nConvertPts = 0;
	};

	extern "C"
	{
		// return value: 0: the same format
		AVEDITOR_API int CompareCodecFormat(AVCodecContext* n_InputCodecContext,
			AVCodecContext* n_OutputCodecContext);
	}

	//////////////////////////////////////////////////////////////////////////
	/// </summary>
	class AVEDITOR_API CVideoConvert : public IFrameConvert
	{
	public:
		CVideoConvert();
		~CVideoConvert();

		void Init(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext) override;

		/*
		* Do with n_Frame
		* return value
		*	0: nothing to do
		*	1: convert the n_Frame
		*/
		int Process(AVFrame* n_Frame) override;

		// Is the converter valid
		const bool IsValid() const override;

		void Release() override;

	protected:
		// Crete converter for decoded frame
		virtual void CreateConverter(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext);

		// Convert the video frame
		void Converting(AVFrame* n_Frame);

	protected:
		// Converter
		FCale			m_Cale;
	};


	//////////////////////////////////////////////////////////////////////////
	class AVEDITOR_API CAudioConvert : public IFrameConvert
	{
	public:
		CAudioConvert();
		~CAudioConvert();

		void Init(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext) override;

		/*
		* Do with n_Frame
		* return value
		*	0: nothing to do
		*	1: convert the n_Frame
		*/
		int Process(AVFrame* n_Frame) override;

		// Is the converter valid
		const bool IsValid() const override;

		void SetAudioFifo(FAudioFifo* n_AudioFifo) override;

		// If all AVFrame are read from the queue, we should pop the last samples 
		// from AVAudioFifo buffer
		void CleanCache() override;

		void Release() override;

	protected:
		// Crete converter for decoded frame
		virtual void CreateConverter(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext);

		// Create Audio Fifo buffer
		void CreateAudioFifo(FCodecContext* n_InputCodecContext,
			FCodecContext* n_OutputCodecContext);

		// Convert the video frame
		void Converting(AVFrame* n_Frame);

		// Pop frame from AVAudioFifo buffer
		void PopFromFifo();

		AVFrame* AllocFrame(int n_nFrameSize);

	protected:
		// Converter
		FResample		m_Resample;

		// Audio FIFO buffer
		FAudioFifo*		m_AudioFifo = nullptr;
	};
}
