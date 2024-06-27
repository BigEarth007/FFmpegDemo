#include "pch.h"
#include "Convert.h"


namespace aveditor
{
	IFrameConvert::IFrameConvert()
	{
	}

	IFrameConvert::~IFrameConvert()
	{
		Release();
	}

	void IFrameConvert::Init(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		Release();

		ThrowExceptionExpr(!n_InputCodecContext || !n_InputCodecContext->m_Context, 
			"Invalid input codec.\n");
		ThrowExceptionExpr(!n_OutputCodecContext || !n_OutputCodecContext->m_Context, 
			"Invalid output codec.\n");

		m_OutputCodecContext = n_OutputCodecContext->m_Context;
	}

	int IFrameConvert::Process(AVFrame* n_Frame)
	{
		return 0;
	}

	void IFrameConvert::SetAudioFifo(FAudioFifo* n_AudioFifo)
	{

	}

	void IFrameConvert::CleanCache()
	{

	}

	const bool IFrameConvert::IsValid() const
	{
		return false;
	}

	void IFrameConvert::SetFinishedCallback(std::function<int(AVFrame*)> n_func)
	{
		m_funcFinished = n_func;
	}

	void IFrameConvert::SetConvertPts(const int64_t n_nPts)
	{
		m_nConvertPts = n_nPts;
	}

	const int64_t IFrameConvert::GetConvertPts() const
	{
		return m_nConvertPts;
	}

	void IFrameConvert::Release()
	{
	}

	void IFrameConvert::FinishedConvert(AVFrame* n_Frame)
	{
		if (m_funcFinished) m_funcFinished(n_Frame);
	}

	AVEDITOR_API int CompareCodecFormat(AVCodecContext* n_InputCodecContext, 
		AVCodecContext* n_OutputCodecContext)
	{
		int ret = -1;
		if (n_InputCodecContext->codec_type != n_OutputCodecContext->codec_type)
			return ret;

		if (n_InputCodecContext->codec_id != n_OutputCodecContext->codec_id)
			return ret;

		if (n_InputCodecContext->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			if (n_InputCodecContext->width == n_OutputCodecContext->width &&
				n_InputCodecContext->height == n_OutputCodecContext->height &&
				n_InputCodecContext->pix_fmt == n_OutputCodecContext->pix_fmt)
			{
				ret = 0;
			}
		}
		else if (n_InputCodecContext->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			if (n_InputCodecContext->sample_fmt == n_OutputCodecContext->sample_fmt &&
				n_InputCodecContext->sample_rate == n_OutputCodecContext->sample_rate &&
				n_InputCodecContext->ch_layout.nb_channels ==
				n_OutputCodecContext->ch_layout.nb_channels)
			{
				ret = 0;
			}
		}

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	CVideoConvert::CVideoConvert()
	{
	}

	CVideoConvert::~CVideoConvert()
	{
		Release();
	}

	void CVideoConvert::Init(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		IFrameConvert::Init(n_InputCodecContext, n_OutputCodecContext);

		CreateConverter(n_InputCodecContext, n_OutputCodecContext);
	}

	int CVideoConvert::Process(AVFrame* n_Frame)
	{
		if (!n_Frame || !m_Cale.m_Context)
		{
			FinishedConvert(n_Frame);
			return 0;
		}

		Converting(n_Frame);

		return 1;
	}

	const bool CVideoConvert::IsValid() const
	{
		return m_Cale.m_Context;
	}

	void CVideoConvert::Release()
	{
		m_Cale.Release();
	}

	void CVideoConvert::CreateConverter(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		if (!n_OutputCodecContext) return;

		AVCodecContext* Input = n_InputCodecContext->m_Context;
		AVCodecContext* Output = n_OutputCodecContext->m_Context;

		if (Input->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			if (Input->codec_id != Output->codec_id ||
				Input->width != Output->width ||
				Input->height != Output->height ||
				n_OutputCodecContext->GetRealPixelFormat() != Output->pix_fmt)
			{
				// Maybe it's a hardware decoder
				AVPixelFormat PixelFormat = Input->pix_fmt;
				if (Input->hw_device_ctx)
				{
					if (*(AVPixelFormat*)Input->opaque ==
						AVPixelFormat::AV_PIX_FMT_D3D11)
						PixelFormat = AVPixelFormat::AV_PIX_FMT_NV12;
				}

				m_Cale.Alloc(
					Input->width,
					Input->height,
					PixelFormat,
					Output->width,
					Output->height,
					Output->pix_fmt);
			}
		}
	}

	void CVideoConvert::Converting(AVFrame* n_Frame)
	{
		if (!m_Cale.m_Context) return;

		AVFrame* Frame = FFrame::VideoFrame(
			m_Cale.GetOutputWidth(),
			m_Cale.GetOutputHeight(),
			m_Cale.GetOutputPixelFormat());

		m_Cale.Cale((const uint8_t**)n_Frame->data, n_Frame->linesize,
			Frame->data, Frame->linesize);

		Frame->pts = n_Frame->pts;
		m_nConvertPts = Frame->pts;
		//LogInfo("Video frame: %zd.\n", Frame->pts);

		FinishedConvert(Frame);
	}

	//////////////////////////////////////////////////////////////////////////
	CAudioConvert::CAudioConvert()
	{
	}

	CAudioConvert::~CAudioConvert()
	{
		Release();
	}

	void CAudioConvert::Init(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		IFrameConvert::Init(n_InputCodecContext, n_OutputCodecContext);

		CreateConverter(n_InputCodecContext, n_OutputCodecContext);
		CreateAudioFifo(n_InputCodecContext, n_OutputCodecContext);
	}

	int CAudioConvert::Process(AVFrame* n_Frame)
	{
		if (!n_Frame || 
			(!m_Resample.m_Context && (!m_AudioFifo || !m_AudioFifo->m_Context)))
		{
			FinishedConvert(n_Frame);
			return 0;
		}

		Converting(n_Frame);

		return 1;
	}

	const bool CAudioConvert::IsValid() const
	{
		return m_Resample.m_Context || (m_AudioFifo && m_AudioFifo->m_Context);
	}

	void CAudioConvert::SetAudioFifo(FAudioFifo* n_AudioFifo)
	{
		m_AudioFifo = n_AudioFifo;
	}

	void CAudioConvert::CleanCache()
	{
		while (m_AudioFifo && m_AudioFifo->Size() > 0)
		{
			PopFromFifo();
		}
	}

	void CAudioConvert::Release()
	{
		m_Resample.Release();
	}

	void CAudioConvert::CreateConverter(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		if (!n_OutputCodecContext) return;

		AVCodecContext* Input = n_InputCodecContext->m_Context;
		AVCodecContext* Output = n_OutputCodecContext->m_Context;

		if (Input->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			if (Input->codec_id != Output->codec_id ||
				Input->sample_fmt != Output->sample_fmt ||
				Input->sample_rate != Output->sample_rate ||
				Input->ch_layout.nb_channels != 
				Output->ch_layout.nb_channels)
			{
				m_Resample.Alloc(Input, Output);
			}
		}
	}

	void CAudioConvert::CreateAudioFifo(FCodecContext* n_InputCodecContext,
		FCodecContext* n_OutputCodecContext)
	{
		if (!n_OutputCodecContext) return;

		AVCodecContext* Input = n_InputCodecContext->m_Context;
		AVCodecContext* Output = n_OutputCodecContext->m_Context;

		if (Input->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO ||
			Input->frame_size == Output->frame_size)
			return;

		if (m_AudioFifo && !m_AudioFifo->m_Context)
			m_AudioFifo->Alloc(Output->sample_fmt,
				Output->ch_layout.nb_channels,
				Output->frame_size);
	}

	void CAudioConvert::Converting(AVFrame* n_Frame)
	{
		if (m_AudioFifo && m_AudioFifo->m_Context)
		{
			// Push into audio fifo buffer
			if (m_Resample.m_Context)
			{
				int nSamples = m_Resample.Cover(n_Frame);
				m_AudioFifo->Push(m_Resample.m_CoverData, nSamples);
			}
			else
			{
				m_AudioFifo->Push(n_Frame->data, n_Frame->nb_samples);
			}

			while (m_AudioFifo->IsReadable())
			{
				PopFromFifo();
			}
		}
		else
		{
			if (!m_Resample.m_Context) return;

			AVFrame* Frame = AllocFrame(m_OutputCodecContext->frame_size);

			m_Resample.Cover((const uint8_t**)n_Frame->extended_data,
				n_Frame->nb_samples, Frame->data, Frame->nb_samples);

			FinishedConvert(Frame);
		}
	}

	void CAudioConvert::PopFromFifo()
	{
		int nSampleSize = m_AudioFifo->NextSampleCount();
		if (nSampleSize <= 0) return;

		AVFrame* Frame = AllocFrame(nSampleSize);

		m_AudioFifo->Pop(Frame);

		FinishedConvert(Frame);
	}

	AVFrame* CAudioConvert::AllocFrame(int n_nFrameSize)
	{
		AVFrame* Frame = FFrame::AudioFrame(
			n_nFrameSize,
			m_OutputCodecContext->sample_rate,
			m_OutputCodecContext->sample_fmt,
			&m_OutputCodecContext->ch_layout);

		Frame->pts = m_nConvertPts;
		m_nConvertPts += n_nFrameSize;

		//LogInfo("Audio frame: %zd.\n", Frame->pts);

		return Frame;
	}

}
