#pragma once
/*
* The basic class of 4 stage(demultiplex, decode, encode, multiplex)
*/


namespace aveditor
{
	class CEditor;
	class AVEDITOR_API IStage : public Thread
	{
	public:
		IStage() = default;
		IStage(CEditor* n_Editor);
		virtual ~IStage();

		void BaseInit(CEditor* n_Editor);

		CAVObject* GetAVObject() const;

		// Pause Thread
		void SetPause(bool n_bPause);

		void StageSleep();

		// Sleep while pause
		void PauseSleep(int n_nMillisecond = kSleepDelay * 20);

		virtual void Release();

	protected:
		CEditor*	m_Editor = nullptr;

		CAVObject*	m_AVObject = nullptr;

		bool		m_bPause = false;
	};
}
