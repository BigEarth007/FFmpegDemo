#pragma once
/*
* The basic class of 4 stage(demultiplex, decode, encode, multiplex)
*/


namespace aveditor
{
	class AVEDITOR_API IStage : public Thread
	{
	public:
		IStage() = default;
		IStage(FCache& n_Cache, const int& n_nPrefix, 
			const EStreamType& n_eStreamType);
		virtual ~IStage();

		void BaseInit(FCache& n_Cache, const int& n_nPrefix,
			const EStreamType& n_eStreamType);

		int GetContextIndex();

		void SetMaxCacheSize(unsigned int n_nMaxCacheSize);

		// If touch max size, sleep 
		void ConsumeCache(const int& n_nKey);

		void StageSleep();

	protected:
		// Buffer queues
		FCache*			m_Cache = nullptr;

		// Max cache size for each stream
		unsigned int	m_nMaxCacheSize = 80;

		int				m_nCurrentPrefix = -1;
		int				m_nPreviousPrefix = -1;
		EStreamType		m_eStreamType = EStreamType::EST_Max;
	};
}
