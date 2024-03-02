#include "pch.h"
#include "SDLPlayer.h"


namespace aveditor
{
	CSDLPlayer::CSDLPlayer(FCache& n_Cache, const int& n_nPrefix)
		: IPlayer(n_Cache, n_nPrefix)
	{
	}

	CSDLPlayer::~CSDLPlayer()
	{
		Release();
	}

	void CSDLPlayer::Init(const unsigned int n_nFlags, 
		const unsigned int n_nFrameDuration)
	{
		m_Sdl.Init(n_nFlags);
		m_nFrameDuration = n_nFrameDuration;
	}

	void CSDLPlayer::InitAudio(AVCodecContext* n_CodecContext)
	{
		m_Sdl.InitAudio(n_CodecContext, AudioCallback, this);
		m_CodecContext = n_CodecContext;
	}

	void CSDLPlayer::InitVideo(const char* n_szTitle, const int n_nWidth, 
		const int n_nHeight, const unsigned int n_nFlags)
	{
		m_Sdl.InitVideo(n_szTitle, n_nWidth, n_nHeight, n_nFlags);
	}

	void CSDLPlayer::InitVideo(const void* n_WinId)
	{
		m_Sdl.InitVideo(n_WinId);
	}

	void CSDLPlayer::Play()
	{
		IPlayer::Play();
		m_Sdl.Play();
	}

	void CSDLPlayer::Pause()
	{
		IPlayer::Pause();
		m_Sdl.Pause();
	}

	int CSDLPlayer::OnEvent()
	{
		return SdlEvent();
	}

	void CSDLPlayer::VideoFrameArrived(const AVFrame* n_Frame)
	{
		m_Sdl.UpdateYUV((AVFrame*)n_Frame, 0);
	}


	void CSDLPlayer::Release()
	{
		m_Sdl.Release();

		m_nFrameDuration = 40;
	}

	void CSDLPlayer::AudioCallback(void* n_UserData, unsigned char* n_nStream, int n_nLen)
	{
		CSDLPlayer* Player = (CSDLPlayer*)n_UserData;
		if (Player)
			Player->AudioProc(n_nStream, n_nLen);
	}

	void CSDLPlayer::AudioProc(unsigned char* n_nStream, int n_nLen)
	{
		GetAudioFrame([this, &n_nStream, &n_nLen](const AVFrame* n_Frame) {
				m_Sdl.UpdateAudio((AVFrame*)n_Frame, n_nStream, n_nLen);
			});
	}

	int CSDLPlayer::SdlEvent()
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
