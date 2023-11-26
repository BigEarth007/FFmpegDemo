#include "pch.h"
#include "Player.h"


namespace aveditor
{
	CPlayer::CPlayer(FCache& n_Cache, const int& n_nPrefix)
		: IStage(n_Cache, n_nPrefix, EStreamType::EST_Max)
	{
	}

	CPlayer::~CPlayer()
	{
		Release();
	}

	void CPlayer::Init(const unsigned int n_nFlags, 
		const unsigned int n_nFrameDuration)
	{
		m_Sdl.Init(n_nFlags);
		m_nFrameDuration = n_nFrameDuration;
	}

	void CPlayer::InitAudio(AVCodecContext* n_CodecContext)
	{
		m_Sdl.InitAudio(n_CodecContext, AudioCallback, this);
		m_CodecContext = n_CodecContext;
	}

	void CPlayer::InitVideo(const char* n_szTitle, const int n_nWidth, 
		const int n_nHeight, const unsigned int n_nFlags)
	{
		m_Sdl.InitVideo(n_szTitle, n_nWidth, n_nHeight, n_nFlags);
	}

	void CPlayer::InitVideo(const void* n_WinId)
	{
		m_Sdl.InitVideo(n_WinId);
	}

	void CPlayer::Play()
	{
		m_Sdl.Play();
	}

	void CPlayer::Pause()
	{
		m_Sdl.Pause();
	}

	void CPlayer::Run()
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

		while (!IsStop() && qVideo)
		{
			ret = SdlEvent();
			if (ret < 0) break;
			if (m_Sdl.IsPause())
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

			nDelay = 0;

			ret = m_Cache->Pop(qVideo, Frame);
			if (ret < 0) continue;

			if (!Frame) break;

			m_Sdl.UpdateYUV(Frame, 0);

			av_frame_free(&Frame);
		}

		Pause();
		Thread::Run();
	}

	void CPlayer::Release()
	{
		m_Sdl.Release();

		m_nFrameDuration = 40;
	}

	void CPlayer::AudioCallback(void* n_UserData, unsigned char* n_nStream, int n_nLen)
	{
		CPlayer* Player = (CPlayer*)n_UserData;
		if (Player)
			Player->AudioProc(n_nStream, n_nLen);
	}

	void CPlayer::AudioProc(unsigned char* n_nStream, int n_nLen)
	{
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		int			ret = 0;
		AVFrame*	Frame = nullptr;

		ret = m_Cache->Pop(m_qAudio, Frame);
		if (ret < 0) return;

		m_Sdl.UpdateAudio(Frame, n_nStream, n_nLen);

		av_frame_free(&Frame);
	}

	int CPlayer::SdlEvent()
	{
		int ret = 0;
		SDL_Event Event;

		SDL_PumpEvents();

		if (SDL_PeepEvents(&Event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 1)
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				ret = -1;
				break;
			case SDL_KEYUP:
				if (Event.key.keysym.sym == SDLK_SPACE)
				{
					if (m_Sdl.IsPause())
						Play();
					else
						Pause();
				}
				break;
			default:
				break;
			}
		}

		return ret;
	}

}
