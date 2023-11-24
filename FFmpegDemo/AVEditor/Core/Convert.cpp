#include "pch.h"
#include "Convert.h"


namespace aveditor
{
	CConvert::CConvert()
	{
	}

	CConvert::~CConvert()
	{
		Release();
	}

	void CConvert::Init(FCodecContext& n_InputCodecContext, 
		FCodecContext& n_OutputCodecContext)
	{
		Release();

		m_OutputCodecContext = n_OutputCodecContext.m_Context;
		CreateConverter(n_InputCodecContext, n_OutputCodecContext);
		CreateAudioFifo(n_InputCodecContext, n_OutputCodecContext);
	}

	int CConvert::Process(AVFrame* n_Frame, const int& n_nKey)
	{
		if (!m_Cale.m_Context && 
			!m_Resample.m_Context && 
			!m_AudioFifo.m_Context)
			return 0;

		VideoFrame(n_Frame, n_nKey);
		AudioFrame(n_Frame, n_nKey);

		return 1;
	}

	void CConvert::CleanAudioFifo(const int& n_nKey)
	{
		while (m_AudioFifo.Size() > 0)
		{
			PopFromFifo(n_nKey);
		}
	}

	void CConvert::SetFinishedCallback(std::function<int(AVFrame*, const int&)> n_func)
	{
		m_funcFinished = n_func;
	}

	void CConvert::Release()
	{
		m_Cale.Release();
		m_Resample.Release();
		m_AudioFifo.Release();
	}

	void CConvert::CreateConverter(FCodecContext& n_InputCodecContext,
		FCodecContext& n_OutputCodecContext)
	{
		if (!n_OutputCodecContext.m_Context) return;

		AVCodecContext* InputContext = n_InputCodecContext.m_Context;
		AVCodecContext* OutputContext = n_OutputCodecContext.m_Context;

		if (OutputContext->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			// Maybe it's a hardware decoder
			AVPixelFormat PixelFormat = InputContext->pix_fmt;
			if (InputContext->hw_device_ctx)
			{
				AVHWFramesConstraints* Constraints =
					av_hwdevice_get_hwframe_constraints(
						InputContext->hw_device_ctx, nullptr);

				if (Constraints->valid_hw_formats[0] == AVPixelFormat::AV_PIX_FMT_D3D11)
					PixelFormat = AVPixelFormat::AV_PIX_FMT_NV12;

				av_hwframe_constraints_free(&Constraints);
			}

			if (InputContext->width != OutputContext->width ||
				InputContext->height != OutputContext->height ||
				PixelFormat != OutputContext->pix_fmt)
			{
				m_Cale.Alloc(
					InputContext->width, InputContext->height, PixelFormat, 
					OutputContext->width, OutputContext->height, OutputContext->pix_fmt);
			}
		}
		else if (OutputContext->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			if (InputContext->sample_fmt != OutputContext->sample_fmt ||
				InputContext->sample_rate != OutputContext->sample_rate ||
				InputContext->ch_layout.nb_channels != OutputContext->ch_layout.nb_channels)
			{
				m_Resample.Alloc(InputContext, OutputContext);
			}
		}
	}

	void CConvert::CreateAudioFifo(FCodecContext& n_InputCodecContext,
		FCodecContext& n_OutputCodecContext)
	{
		if (!n_OutputCodecContext.m_Context) return;

		AVCodecContext* InputContext = n_InputCodecContext.m_Context;
		AVCodecContext* OutputContext = n_OutputCodecContext.m_Context;

		if (OutputContext->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO ||
			InputContext->frame_size == OutputContext->frame_size)
			return;

		m_AudioFifo.Alloc(OutputContext->sample_fmt, 
			OutputContext->ch_layout.nb_channels, OutputContext->frame_size);
	}

	void CConvert::VideoFrame(AVFrame* n_Frame, const int& n_nKey)
	{
		if (!m_Cale.m_Context) return;

		AVFrame* Frame = FFrame::VideoFrame(
			m_Cale.GetOutputWidth(), 
			m_Cale.GetOutputHeight(), 
			m_Cale.GetOutputPixelFormat());

		m_Cale.Cale((const uint8_t**)n_Frame->data, n_Frame->linesize,
			Frame->data, Frame->linesize);

		Frame->pts = n_Frame->pts;
		//LogInfo("Video frame: %zd.\n", Frame->pts);

		FinishedConvert(Frame, n_nKey);
	}

	void CConvert::AudioFrame(AVFrame* n_Frame, const int& n_nKey)
	{
		if (m_AudioFifo.m_Context)
		{
			// Push into audio fifo buffer
			if (m_Resample.m_Context)
			{
				int nSamples = m_Resample.Cover(n_Frame);
				m_AudioFifo.Push(m_Resample.m_CoverData, nSamples);
			}
			else
			{
				m_AudioFifo.Push(n_Frame->data, n_Frame->nb_samples);
			}

			while (m_AudioFifo.IsReadable())
			{
				PopFromFifo(n_nKey);
			}
		}
		else
		{
			if (!m_Resample.m_Context) return;

			AVFrame* Frame = FFrame::AudioFrame(
				m_OutputCodecContext->frame_size, 
				m_Resample.GetOutputSampleRate(),
				m_Resample.GetOutputSampleFormat(), 
				&m_OutputCodecContext->ch_layout);

			Frame->pts = m_nAudioFramePts;
			m_nAudioFramePts += Frame->nb_samples;

			//LogInfo("Audio frame: %zd.\n", Frame->pts);

			m_Resample.Cover((const uint8_t**)n_Frame->extended_data,
				n_Frame->nb_samples, Frame->data, Frame->nb_samples);

			FinishedConvert(Frame, n_nKey);
		}
	}

	void CConvert::PopFromFifo(const int& n_nKey)
	{
		int nSampleSize = m_AudioFifo.NextSampleCount();
		if (nSampleSize <= 0) return;

		AVFrame* Frame = FFrame::AudioFrame(
			nSampleSize,
			m_OutputCodecContext->sample_rate,
			m_OutputCodecContext->sample_fmt,
			&m_OutputCodecContext->ch_layout);

		m_AudioFifo.Pop(Frame);
		Frame->pts = m_nAudioFramePts;
		m_nAudioFramePts += nSampleSize;

		//LogInfo("Audio frame: %zd.\n", Frame->pts);

		FinishedConvert(Frame, n_nKey);
	}

	void CConvert::FinishedConvert(AVFrame* n_Frame, const int& n_nKey)
	{
		if (m_funcFinished) m_funcFinished(n_Frame, n_nKey);
	}

}