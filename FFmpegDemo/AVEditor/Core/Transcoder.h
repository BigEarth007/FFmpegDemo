#pragma once
/*
* Combining encoding stage and decoding stage
*/


namespace aveditor
{
	class AVEDITOR_API CTranscoder : public CDecoder
	{
	public:
		CTranscoder(FCache& n_Cache, const int& n_nPrefix,
			const EStreamType& n_eStreamType);
		~CTranscoder();

		void Init(FCodecContext& n_InputCodecContext,
			FCodecContext& n_OutputCodecContext);

		virtual void Run();

		virtual void Release();

	protected:
		virtual int FinishedConvert(AVFrame* n_Frame, const int& n_nKey);

	};
}
