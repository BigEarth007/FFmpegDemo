#pragma once


namespace aveditor
{
	class AVEDITOR_API CBaseContext
	{
	public:
		CBaseContext(std::vector<IStage*>& n_vStages,
			FCache& n_Cache, const int n_nContextIndex);
		virtual ~CBaseContext();

		FFormatContext& GetContext();

		virtual CDemuxer* CreateDemuxer() { return nullptr; }

		virtual void CreateDecoder() {}

		virtual void CreateEncoder() {}

		virtual CMuxer* CreateMuxer() { return nullptr; }

		void Release();

	protected:
		FFormatContext			m_Context;

		std::vector<IStage*>*	m_vStages;
		// Buffer Queue
		FCache*					m_Cache = nullptr;
		// Index of this format context
		int						m_nContextIndex = 0;
	};
}
