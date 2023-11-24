#pragma once


namespace aveditor
{
	class AVEDITOR_API CEncoder : public IStage
	{
	public:
		CEncoder(FCache& n_Cache, const int& n_nPrefix,
			const EStreamType& n_eStreamType);
		~CEncoder();

		void Init(AVCodecContext* n_CodecContext);

		virtual void Run();

		void Release();

	protected:
		// Encode codec context
		FCodecContext	m_OutputCodecContext;
	};

}
