#pragma once


namespace aveditor
{
	class AVEDITOR_API COutputContext : public CBaseContext
	{
	public:
		COutputContext(std::vector<IStage*>& n_vStages,
			FCache& n_Cache, const int n_nContextIndex);
		virtual ~COutputContext();

		FFormatContext& AllocOutputFile(const std::string& n_sFileName,
			const AVOutputFormat* n_OutputFormat = nullptr,
			const char* n_szFormatName = nullptr);

		void OpenOutputFile();
		void CloseOutputFile();

		virtual CMuxer* CreateMuxer();

	};
}