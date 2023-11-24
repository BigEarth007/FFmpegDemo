#pragma once


namespace aveditor
{
	struct AVEDITOR_API FFrame
	{
		FFrame() = default;
		~FFrame();

		// Alloc packet memory
		AVFrame* Alloc();
		// Release 
		void Release();

		// Copy frame. No need to alloc m_Frame
		AVFrame* Clone(const AVFrame* n_Frame);
		// Move frame ref
		void MoveRef(AVFrame* n_Frame);
		// Unref frame
		void UnRef();
		// Make writable
		void MakeWritable();

		// Alloc data buffer for video frame
		void AllocVideoBuffer(const int n_nWidth, const int n_nHeight, 
			const AVPixelFormat n_ePixelFormat);
		void AllocVideoBuffer(const AVCodecContext* n_CodecContext);
		void AllocVideoBuffer(const FCodecContext& n_CodecContext);
		// Alloc data buffer for audio frame
		void AllocAudioBuffer(const int n_nSamples, const int n_nSampleRate, 
			const AVSampleFormat n_eSampleFormat, const AVChannelLayout* n_ChannelLayout);
		void AllocAudioBuffer(const AVCodecContext* n_CodecContext);
		void AllocAudioBuffer(const FCodecContext& n_CodecContext);

		// Alloc frame for video, alloc data buffer for writing
		static AVFrame* VideoFrame(const int n_nWidth, const int n_nHeight, 
			const AVPixelFormat n_ePixelFormat);
		// Alloc frame for audio, alloc data buffer for writing
		static AVFrame* AudioFrame(const int n_nSamples, const int n_nSampleRate,
			const AVSampleFormat n_eSampleFormat, const AVChannelLayout* n_ChannelLayout);

		AVFrame* m_Frame = nullptr;
	};
}
