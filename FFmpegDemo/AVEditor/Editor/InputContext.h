#pragma once


namespace aveditor
{
	class AVEDITOR_API CInputContext : public CBaseContext
	{
	public:
		CInputContext(std::vector<IStage*>& n_vStages,
			FCache& n_Cache, const int n_nContextIndex);
		virtual ~CInputContext();

		FFormatContext& OpenInputFile(const std::string& n_sFileName,
			const AVInputFormat* n_InputFormat = nullptr,
			AVDictionary* n_Options = nullptr);

		virtual CDemuxer* CreateDemuxer(FFormatContext& n_OutputContext);

		virtual void CreateDecoder(FFormatContext& n_OutputContext);
		virtual void CreateEncoder(FFormatContext& n_OutputContext);
		// Equal CreateDecoder + CreateEncoder
		virtual void CreateTranscoder(FFormatContext& n_OutputContext);

		virtual IPlayer* CreatePlayer(IPlayer* n_Player = nullptr);
	};
}
