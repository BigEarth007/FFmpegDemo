#pragma once


namespace aveditor
{
	class AVEDITOR_API CDemuxer : public IStage
	{
	public:
		CDemuxer(FCache& n_Cache, const int& n_nPrefix);
		~CDemuxer();

		// Set the input file context
		void Init(FFormatContext& n_InputContext, FFormatContext* n_OutputContext = nullptr);
		// Release
		void Release();

		// Set time section; it's second;
		// const double n_dStart: start timestamp
		// const double n_dDuration: time duration, 0 means to the end
		void SelectSection(const double n_dStart, const double n_dDuration = 0);

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
	};
}
