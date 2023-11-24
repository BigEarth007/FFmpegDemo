#pragma once


namespace aveditor
{
	struct AVEDITOR_API FSdl
	{
		FSdl() = default;
		~FSdl();

		// Init SDL
		void Init(const unsigned int n_nFlags);

		// Init audio parameter
		void InitAudio(AVCodecContext* n_CodecContext,
			SDL_AudioCallback n_Cb, void* n_Param);

		// Create video render window
		void InitVideo(const char* n_szTitle, const int n_nWidth, 
			const int n_nHeight, const unsigned int n_nFlags);

		// Create video render window
		// n_WinId: MFC window HWND or QT winID
		void InitVideo(const void* n_WinId);

		// Input YUV frame to render on the window
		void UpdateYUV(AVFrame* n_Frame, const unsigned int n_nDelay);
		// Input audio frame to play audio
		// It's called by SDL_AudioCallback in InitAudio() function
		void UpdateAudio(AVFrame* n_Frame, unsigned char* n_Stream, int n_nLen);

		// Play
		void Play();
		// Pause
		void Pause();
		// Is pause now
		bool IsPause();

		void Release();

		SDL_Window*		m_Window = nullptr;
		SDL_Renderer*	m_Renderer = nullptr;
		SDL_Texture*	m_Texture = nullptr;

		SDL_Rect		m_Rect = { 0 };

	protected:
		// If the input context is planar
		int				m_nPlanar = 0;
		// 0: play; 1: pause
		int				m_nPause = 0;
	};
}
