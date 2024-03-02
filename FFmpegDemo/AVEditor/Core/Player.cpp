#include "pch.h"
#include "Player.h"


namespace aveditor
{
	IPlayer::IPlayer(FCache& n_Cache, const int& n_nPrefix)
		: IStage(n_Cache, n_nPrefix, EStreamType::EST_Max)
	{
	}

	IPlayer::~IPlayer()
	{
		Release();
	}

	void IPlayer::Init(const unsigned int n_nFlags, 
		const unsigned int n_nFrameDuration)
	{
		m_nFrameDuration = n_nFrameDuration;
	}

	void IPlayer::Start()
	{
		IStage::Start();
		Play();
	}

	void IPlayer::Play()
	{
		m_eState = EPlayState::EPS_Play;
	}

	void IPlayer::Pause()
	{
		m_eState = EPlayState::EPS_Pause;
	}

	int IPlayer::OnEvent()
	{
		return 0;
	}

	void IPlayer::Run()
	{
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		int			ret = 0;
		int			nDelay = 0;
		int			nKeyVideo = m_nCurrentPrefix + (int)EStreamType::EST_Video;
		int			nKeyAudio = m_nCurrentPrefix + (int)EStreamType::EST_Audio;
		AVFrame*	Frame = nullptr;

		CQueueItem* qVideo = m_Cache->GetBufferQueue(nKeyVideo);
		m_qAudio = m_Cache->GetBufferQueue(nKeyAudio);

		PreRun();

		while (!IsStop() && qVideo && m_eState != EPlayState::EPS_Stop)
		{
			if (ret < OnEvent()) break;
			if (m_eState == EPlayState::EPS_Pause)
			{
				Sleep(kSleepDelay);
				continue;
			}

			if (nDelay < m_nFrameDuration)
			{
				nDelay += kSleepDelay;
				Sleep(kSleepDelay);
				continue;
			}

			ret = m_Cache->Pop(qVideo, Frame);
			if (ret < 0) continue;

			if (!Frame) break;

			VideoFrameArrived(Frame);

			av_frame_free(&Frame);

			nDelay = 0;
		}

		Pause();
		Thread::Run();
	}

	void IPlayer::VideoFrameArrived(const AVFrame* n_Frame)
	{
	}

	int IPlayer::GetAudioFrame(std::function<void(const AVFrame*)> n_func)
	{
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		int			ret = 0;
		AVFrame*	Frame = nullptr;

		ret = m_Cache->Pop(m_qAudio, Frame);
		if (ret >= 0)
		{
			if (n_func) n_func(Frame);
		}

		av_frame_free(&Frame);

		return ret;
	}

	void IPlayer::Release()
	{
		m_nFrameDuration = 40;
	}

}
