#pragma once
/*
* Play media with SDL library, it's just test, and Audio-video synchronization 
* is not achieved
*/


namespace aveditor
{
	class AVEDITOR_API CSDLPlayer : public IPlayer
	{
	public:
		CSDLPlayer(FCache& n_Cache, const int& n_nPrefix);
		~CSDLPlayer();

		void Init(const unsigned int n_nFlags,
			const unsigned int n_nFrameDuration);

		void InitAudio(AVCodecContext* n_CodecContext);

		void InitVideo(const char* n_szTitle, const int n_nWidth,
			const int n_nHeight, const unsigned int n_nFlags);

		void InitVideo(const void* n_WinId);

		void Play();
		void Pause();

		virtual int OnEvent();
		virtual void VideoFrameArrived(const AVFrame* n_Frame);

		void Release();

	protected:
		// Audio callback
		static void AudioCallback(void* n_UserData,
			unsigned char* n_nStream, int n_nLen);

		// Audio callback proc
		void AudioProc(unsigned char* n_nStream, int n_nLen);

		// Sdl Event
		int SdlEvent();

	protected:
		// Sdl object
		FSdl	m_Sdl;

		CQueueItem*		m_qAudio = nullptr;

		// Output codec context
		AVCodecContext* m_CodecContext = nullptr;

		// Duration between two frames
		int				m_nFrameDuration = 40;
	};
}
