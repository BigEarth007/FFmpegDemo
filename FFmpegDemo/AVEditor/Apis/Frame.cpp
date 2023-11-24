#include "pch.h"
#include "Frame.h"


namespace aveditor
{
	FFrame::~FFrame()
	{
		Release();
	}

	AVFrame* FFrame::Alloc()
	{
		m_Frame = av_frame_alloc();
		ThrowExceptionExpr(!m_Frame, "Fail to create frame.\n");

		m_Frame->pts = 0;
		m_Frame->pkt_dts = 0;
		m_Frame->duration = 0;

		return m_Frame;
	}

	void FFrame::Release()
	{
		if (m_Frame)
		{
			av_frame_free(&m_Frame);
			m_Frame = nullptr;
		}
	}

	AVFrame* FFrame::Clone(const AVFrame* n_Frame)
	{
		m_Frame = av_frame_clone(n_Frame);
		return m_Frame;
	}

	void FFrame::MoveRef(AVFrame* n_Frame)
	{
		if (!m_Frame) Alloc();
		av_frame_move_ref(m_Frame, n_Frame);
	}

	void FFrame::UnRef()
	{
		if (m_Frame) av_frame_unref(m_Frame);
	}

	void FFrame::MakeWritable()
	{
		int ret = av_frame_make_writable(m_Frame);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to make frame writable.");
	}

	void FFrame::AllocVideoBuffer(const int n_nWidth, const int n_nHeight, 
		const AVPixelFormat n_ePixelFormat)
	{
		if (!m_Frame) Alloc();

		m_Frame->width = n_nWidth;
		m_Frame->height = n_nHeight;
		m_Frame->format = n_ePixelFormat;

		int ret = av_frame_get_buffer(m_Frame, 0);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to alloc buffer for video frame.");
	}

	void FFrame::AllocVideoBuffer(const AVCodecContext* n_CodecContext)
	{
		AllocVideoBuffer(n_CodecContext->width, n_CodecContext->height, 
			n_CodecContext->pix_fmt);
	}

	void FFrame::AllocVideoBuffer(const FCodecContext& n_CodecContext)
	{
		AllocVideoBuffer(n_CodecContext.m_Context);
	}

	void FFrame::AllocAudioBuffer(const int n_nSamples, const int n_nSampleRate, 
		const AVSampleFormat n_eSampleFormat, const AVChannelLayout* n_ChannelLayout)
	{
		if (!m_Frame) Alloc();

		m_Frame->nb_samples = n_nSamples;
		m_Frame->sample_rate = n_nSampleRate;
		m_Frame->format = n_eSampleFormat;

		int ret = av_channel_layout_copy(&m_Frame->ch_layout, n_ChannelLayout);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to copy channel layout for audio frame.");

		ret = av_frame_get_buffer(m_Frame, 0);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to alloc buffer for audio frame.");
	}

	void FFrame::AllocAudioBuffer(const AVCodecContext* n_CodecContext)
	{
		AllocAudioBuffer(n_CodecContext->frame_size, n_CodecContext->sample_rate, 
			n_CodecContext->sample_fmt, &n_CodecContext->ch_layout);
	}

	void FFrame::AllocAudioBuffer(const FCodecContext& n_CodecContext)
	{
		AllocAudioBuffer(n_CodecContext.m_Context);
	}

	AVFrame* FFrame::VideoFrame(const int n_nWidth, const int n_nHeight, 
		const AVPixelFormat n_ePixelFormat)
	{
		AVFrame* Frame = av_frame_alloc();
		ThrowExceptionExpr(!Frame, "Fail to create frame.\n");

		Frame->pts = 0;
		Frame->pkt_dts = 0;
		Frame->duration = 0;

		Frame->width = n_nWidth;
		Frame->height = n_nHeight;
		Frame->format = n_ePixelFormat;

		int ret = av_frame_get_buffer(Frame, 0);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to alloc buffer for video frame.");

		ret = av_frame_make_writable(Frame);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to make frame writable.");

		return Frame;
	}

	AVFrame* FFrame::AudioFrame(const int n_nSamples, const int n_nSampleRate, 
		const AVSampleFormat n_eSampleFormat, const AVChannelLayout* n_ChannelLayout)
	{
		AVFrame* Frame = av_frame_alloc();
		ThrowExceptionExpr(!Frame, "Fail to create frame.\n");

		Frame->pts = 0;
		Frame->pkt_dts = 0;
		Frame->duration = 0;

		Frame->nb_samples = n_nSamples;
		Frame->sample_rate = n_nSampleRate;
		Frame->format = n_eSampleFormat;

		int ret = av_channel_layout_copy(&Frame->ch_layout, n_ChannelLayout);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to copy channel layout for audio frame.");

		ret = av_frame_get_buffer(Frame, 0);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to alloc buffer for audio frame.");

		ret = av_frame_make_writable(Frame);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to make frame writable.");

		return Frame;
	}

}