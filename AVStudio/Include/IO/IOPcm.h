#ifndef __IOPCM_H__
#define __IOPCM_H__
#include "IO/IOHandle.h"
#include "Apis/Frame.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
};
#endif


namespace avstudio
{
	// For writing PCM data
	class IIOPcm
	{
	public:
		void SetupInputParameter(AVCodecContext* n_Codecctx);

		int Writing(const AVMediaType n_eMediatype,
			const void* n_Data, const int n_nSize);

		// Fill video frame when write PCM data
		// Caller rewrite this function to fill frame data
		virtual void FillVideoFrame(AVFrame* n_Frame,
			const void* n_Data, const int n_nSize) const;

		// Fill audio frame when write PCM data
		// Caller rewrite this function to fill frame data
		virtual void FillAudioFrame(AVFrame* n_Frame,
			const void* n_Data, const int n_nSize) const;

		// Receive AVFrame here
		virtual int ReceiveData(const AVMediaType n_eMediaType, 
			AVFrame* n_Frame) = 0;

		const int VideoPlanes() const;

		const int BytesPerPixel() const;

		const int IsAudioPlanar() const;

		const int BytesPerSample() const;

		const int ChannelCount() const;

	protected:
		// For video frame
		AVCodecContext* m_vCodecCtx = nullptr;
		AVFrame*		m_vFrame = nullptr;
		int64_t			m_nvFrameIndex = 0;

		// For audio frame
		AVCodecContext* m_aCodecCtx = nullptr;
		AVFrame*		m_aFrame = nullptr;
		int64_t			m_naFrameIndex = 0;
	};

}


#endif // !__IOPCM_H__
