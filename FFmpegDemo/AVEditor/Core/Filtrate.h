#pragma once
/*
* It's function is just mix audio AVFrame
*/


namespace aveditor
{
	class AVEDITOR_API CFiltrate : public IStage
	{
	public:
		CFiltrate(FCache& n_Cache, const int& n_nPrefix,
			const EStreamType& n_eStreamType);
		~CFiltrate();

		void Init(FCodecContext& n_OutputCodecContext);

		virtual void Run();

		void Release();

	protected:
		// Build filter to mix audio AVFrame
		int CreateMixFilter(AVCodecContext* n_OutputCodecContext);

	protected:
		FFilter			m_Filter;
		AVFrame*		m_PopFrame = nullptr;

		AVCodecContext* m_OutputCodecContext = nullptr;
	};
}
