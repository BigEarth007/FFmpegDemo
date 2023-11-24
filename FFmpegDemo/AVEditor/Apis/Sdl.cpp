#include "pch.h"
#include "Sdl.h"


namespace aveditor
{
	FSdl::~FSdl()
	{
		Release();
	}

	void FSdl::Init(const unsigned int n_nFlags)
	{
		int ret = SDL_Init(n_nFlags);
		ThrowExceptionExpr(ret, "Fail to initialize SDL: %s.\n", SDL_GetError());
	}

	void FSdl::InitAudio(AVCodecContext* n_CodecContext, 
		SDL_AudioCallback n_Cb, void* n_Param)
	{
		SDL_AudioSpec AudioSpec = { 0 };

		AudioSpec.freq = n_CodecContext->sample_rate;
		AudioSpec.channels = n_CodecContext->ch_layout.nb_channels;
		AudioSpec.silence = 0;
		AudioSpec.samples = n_CodecContext->frame_size;
		AudioSpec.callback = n_Cb;
		AudioSpec.userdata = n_Param;

		switch (n_CodecContext->sample_fmt)
		{
		case AVSampleFormat::AV_SAMPLE_FMT_S16:
		case AVSampleFormat::AV_SAMPLE_FMT_S16P:
			AudioSpec.format = AUDIO_S16;
			break;
		case AVSampleFormat::AV_SAMPLE_FMT_S32:
		case AVSampleFormat::AV_SAMPLE_FMT_S32P:
			AudioSpec.format = AUDIO_S32;
			break;
		case AVSampleFormat::AV_SAMPLE_FMT_FLT:
		case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
			AudioSpec.format = AUDIO_F32;
			break;
		default:
			break;
		}

		int ret = SDL_OpenAudio(&AudioSpec, nullptr);
		ThrowExceptionExpr(ret, "Fail to open audio: %s.\n", SDL_GetError());

		m_nPlanar = av_sample_fmt_is_planar(n_CodecContext->sample_fmt) + 1;

		Play();
	}

	void FSdl::InitVideo(const char* n_szTitle, const int n_nWidth, const int n_nHeight, 
		const unsigned int n_nFlags)
	{
		if (m_Window) return;

		m_Window = SDL_CreateWindow(n_szTitle,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			n_nWidth,
			n_nHeight,
			n_nFlags);

		ThrowExceptionExpr(!m_Window, "Fail to create window: %s.\n", SDL_GetError());

		m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
		m_Texture = SDL_CreateTexture(m_Renderer,
			SDL_PIXELFORMAT_IYUV,
			SDL_TEXTUREACCESS_STREAMING,
			n_nWidth,
			n_nHeight);

		m_Rect.w = n_nWidth;
		m_Rect.h = n_nHeight;
	}

	void FSdl::InitVideo(const void* n_WinId)
	{
		if (m_Window) return;

		m_Window = SDL_CreateWindowFrom(n_WinId);

		ThrowExceptionExpr(!m_Window, "Fail to create window: %s.\n", SDL_GetError());

		SDL_GetWindowSize(m_Window, &m_Rect.w, &m_Rect.h);

		m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
		m_Texture = SDL_CreateTexture(m_Renderer,
			SDL_PIXELFORMAT_IYUV,
			SDL_TEXTUREACCESS_STREAMING,
			m_Rect.w,
			m_Rect.h);
	}

	void FSdl::UpdateYUV(AVFrame* n_Frame, const unsigned int n_nDelay)
	{
		if (!m_Texture || !m_Renderer || m_nPause || !n_Frame) return;

		SDL_UpdateYUVTexture(m_Texture, &m_Rect,
			n_Frame->data[0], n_Frame->linesize[0],
			n_Frame->data[1], n_Frame->linesize[1],
			n_Frame->data[2], n_Frame->linesize[2]);

		SDL_RenderClear(m_Renderer);
		SDL_RenderCopy(m_Renderer, m_Texture, nullptr, &m_Rect);
		SDL_RenderPresent(m_Renderer);

		SDL_Delay(n_nDelay);
	}

	void FSdl::UpdateAudio(AVFrame* n_Frame, unsigned char* n_Stream, int n_nLen)
	{
		if (m_nPause || !n_Frame) return;

		SDL_memset(n_Stream, 0, n_nLen);

		int nOffset = n_Frame->linesize[0] / m_nPlanar;

		for (size_t i = 0; i < m_nPlanar; i++)
			SDL_MixAudio(n_Stream + nOffset * i, n_Frame->data[i], nOffset, SDL_MIX_MAXVOLUME);
	}

	void FSdl::Play()
	{
		m_nPause = 0;
		SDL_PauseAudio(m_nPause);
	}

	void FSdl::Pause()
	{
		m_nPause = 1;
		SDL_PauseAudio(m_nPause);
	}

	bool FSdl::IsPause()
	{
		return m_nPause == 1;
	}

	void FSdl::Release()
	{
		if (m_Texture) SDL_DestroyTexture(m_Texture);
		if (m_Renderer) SDL_DestroyRenderer(m_Renderer);
		if (m_Window) SDL_DestroyWindow(m_Window);
		SDL_CloseAudio();
		SDL_Quit();

		m_Texture = nullptr;
		m_Renderer = nullptr;
		memset(&m_Rect, 0, sizeof(SDL_Rect));
	}

}
