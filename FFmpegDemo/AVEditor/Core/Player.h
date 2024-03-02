#pragma once
/*
* Play media with SDL library, it's just test, and Audio-video synchronization 
* is not achieved
*/


namespace aveditor
{
	enum class EPlayState
	{
		EPS_Play = 0,
		EPS_Pause,
		EPS_Stop
	};

	class AVEDITOR_API IPlayer : public IStage
	{
	public:
		IPlayer(FCache& n_Cache, const int& n_nPrefix);
		virtual ~IPlayer();

		virtual void Init(const unsigned int n_nFlags,
			const unsigned int n_nFrameDuration);

		virtual void Start();
		virtual void Play();
		virtual void Pause();

		virtual int OnEvent();
		virtual void Run();

		virtual void VideoFrameArrived(const AVFrame* n_Frame);
		int GetAudioFrame(std::function<void(const AVFrame*)> n_func);

		virtual void Release();

	protected:
		CQueueItem*		m_qAudio = nullptr;

		// Duration between two frames
		int				m_nFrameDuration = 40;

		// Play state
		EPlayState		m_eState = EPlayState::EPS_Stop;
	};
}
