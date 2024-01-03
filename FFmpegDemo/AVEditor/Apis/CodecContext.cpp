#include "pch.h"
#include "CodecContext.h"


namespace aveditor
{
	enum AVHWDeviceType hw_priority[] = {
		AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA,
		AVHWDeviceType::AV_HWDEVICE_TYPE_DXVA2,
		//AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA,
		AVHWDeviceType::AV_HWDEVICE_TYPE_QSV,
		AVHWDeviceType::AV_HWDEVICE_TYPE_NONE
	};

	AVPixelFormat GetHwFormat(AVCodecContext* n_CodecContext,
		const AVPixelFormat* n_PixelFormats)
	{
		const AVPixelFormat* p;
		
		for (p = n_PixelFormats; *p != AVPixelFormat::AV_PIX_FMT_NONE; p++)
		{
			if (*p == *(AVPixelFormat*)n_CodecContext->opaque)
				return *p;
		}

		return AVPixelFormat::AV_PIX_FMT_NONE;
	}

	FCodecContext::~FCodecContext()
	{
		Release();
	}

	const AVCodec* FCodecContext::FindDecodeCodec(AVCodecID n_CodecID)
	{
		const AVCodec* Codec = avcodec_find_decoder(n_CodecID);
		ThrowExceptionExpr(!Codec, "Fail to find decoder: %s", 
			avcodec_get_name(n_CodecID));

		return Codec;
	}

	const AVCodec* FCodecContext::FindDecodeCodec(const char* n_szName)
	{
		const AVCodec* Codec = avcodec_find_decoder_by_name(n_szName);
		ThrowExceptionExpr(!Codec, "Fail to find decoder: %s", n_szName);

		return Codec;
	}

	const AVCodec* FCodecContext::FindEncodeCodec(AVCodecID n_CodecID)
	{
		const AVCodec* Codec = nullptr;

		if (IsHwEncodeSupported())
		{
			switch (n_CodecID)
			{
			case AVCodecID::AV_CODEC_ID_H264:
				Codec = FindEncodeCodec("h264_nvenc");
				break;
			case AVCodecID::AV_CODEC_ID_HEVC:
				Codec = FindEncodeCodec("hevc_nvenc");
				break;
			default:
				Codec = avcodec_find_encoder(n_CodecID);
				break;
			}
		}
		else
			Codec = avcodec_find_encoder(n_CodecID);
			
		ThrowExceptionExpr(!Codec, "Fail to find encoder: %s",
			avcodec_get_name(n_CodecID));

		return Codec;
	}

	const AVCodec* FCodecContext::FindEncodeCodec(const char* n_szName)
	{
		const AVCodec* Codec = avcodec_find_encoder_by_name(n_szName);
		ThrowExceptionExpr(!Codec, "Fail to find encoder: %s", n_szName);

		return Codec;
	}

	bool FCodecContext::IsHwEncodeSupported()
	{
		static int nHwEncodeSupported = -1;
		if (nHwEncodeSupported != -1)
			return nHwEncodeSupported == 1;

		const AVCodec* Codec = FindEncodeCodec("h264_nvenc");
		AVCodecContext* Context = avcodec_alloc_context3(Codec);

		Context->width = 640;
		Context->height = 480;
		Context->bit_rate = 128000;
		Context->time_base = { 1, 96000 };
		Context->framerate = { 25, 1 };
		Context->pix_fmt = *Codec->pix_fmts;

		int ret = avcodec_open2(Context, Codec, nullptr);
		if (ret == 0) 
		{
			nHwEncodeSupported = 1;
			avcodec_free_context(&Context);
		}
		else nHwEncodeSupported = 0;

		return nHwEncodeSupported == 1;
	}

	AVCodecContext* FCodecContext::Alloc(const AVCodec* n_Codec)
	{
		ThrowExceptionExpr(!n_Codec, 
			"Invalid parameter: n_Codec is nullptr.\n");

		m_Context = avcodec_alloc_context3(n_Codec);
		ThrowExceptionExpr(!m_Context, 
			"Fail to alloc codec context: %s", n_Codec->name);

		return m_Context;
	}

	void FCodecContext::Release()
	{
		if (m_Context)
			avcodec_free_context(&m_Context);

		if (m_HwDeviceContext)
			av_buffer_unref(&m_HwDeviceContext);
	}

	int FCodecContext::GetHwPixelFormat(const AVCodec* n_Codec, 
		AVHWDeviceType n_eHwDeviceType, AVPixelFormat& n_ePixelFormat)
	{
		int ret = -1;
		n_ePixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;

		if (n_Codec->type != AVMediaType::AVMEDIA_TYPE_VIDEO)
			return ret;

		for (int i = 0;;i++)
		{
			const AVCodecHWConfig* config = avcodec_get_hw_config(n_Codec, i);
			if (!config) break;

			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
				config->device_type == n_eHwDeviceType)
			{
				n_ePixelFormat = config->pix_fmt;
				ret = 0;
				break;
			}
		}

		return ret;
	}

	void FCodecContext::HwInit(AVHWDeviceType n_eHwDeviceType)
	{
		int ret = av_hwdevice_ctx_create(&m_HwDeviceContext, n_eHwDeviceType, 
			nullptr, nullptr, 0);

		ThrowExceptionCodeExpr(ret < 0, ret, "Failed to create specified HW device.\n");
		
		m_Context->hw_device_ctx = av_buffer_ref(m_HwDeviceContext);
	}

	void FCodecContext::CopyCodecParameter(const AVStream* n_Stream)
	{
		ThrowExceptionExpr(!n_Stream, "Invalid parameter: n_Stream is nullptr.\n");

		int ret = avcodec_parameters_to_context(m_Context, n_Stream->codecpar);
		ThrowExceptionCodeExpr(ret < 0, ret, 
			"Fail to copy codec parameters form stream.");
		
		m_Context->time_base = n_Stream->time_base;
	}

	void FCodecContext::CopyCodecParameter(const AVCodecContext* n_CodecContext)
	{
		Alloc(n_CodecContext->codec);

		AVCodecParameters* par = avcodec_parameters_alloc();
		avcodec_parameters_from_context(par, n_CodecContext);

		avcodec_parameters_to_context(m_Context, par);
		avcodec_parameters_free(&par);

		m_Context->time_base = n_CodecContext->time_base;
		m_Context->framerate = n_CodecContext->framerate;
	}

	void FCodecContext::CopyAdditionParameter(const AVStream* n_Stream)
	{
		if (n_Stream->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			if (n_Stream->avg_frame_rate.den == 1)
				m_Context->framerate = n_Stream->avg_frame_rate;
			else
			{
				float num = n_Stream->avg_frame_rate.num * 1.0f / n_Stream->avg_frame_rate.den;
				if (num - (int)num > 0) num += 0.5f;
				m_Context->framerate = { (int)num, 1 };
			}
		}
		else if (n_Stream->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			m_Context->time_base = { 1, m_Context->sample_rate };
		}
	}

	void FCodecContext::Open(AVDictionary** n_Options)
	{
		ThrowExceptionExpr(!m_Context, "You should call function Alloc() first.\n");

		int nIsDecoder = av_codec_is_decoder(m_Context->codec);
		if (nIsDecoder &&
			m_Context->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO &&
			m_Context->pix_fmt != AVPixelFormat::AV_PIX_FMT_YUV422P)
		{
			AVHWDeviceType* priority = hw_priority;
			while (*priority != AVHWDeviceType::AV_HWDEVICE_TYPE_NONE)
			{
				int ret = GetHwPixelFormat(m_Context->codec, *priority, m_eHwPixelFormat);
				if (ret >= 0)
				{
					HwInit(*priority);
					m_Context->thread_count = 0;
					m_Context->get_format = GetHwFormat;
					m_Context->opaque = &m_eHwPixelFormat;
					break;
				}
				priority++;
			}
		}

		int ret = avcodec_open2(m_Context, m_Context->codec, n_Options);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to open codec context.");
	}

	void FCodecContext::Close()
	{
		avcodec_close(m_Context);
	}

	int FCodecContext::DecodePacket(const AVPacket* n_Packet,
		std::function<int(AVFrame* n_Frame)> n_Func)
	{
		ThrowExceptionExpr(!m_Context, "You should call function Alloc() first.\n");

		int ret = avcodec_send_packet(m_Context, n_Packet);
		if (ret < 0) return ret;
		AVFrame* Frame = nullptr;
		AVFrame* SwFrame = nullptr;
		AVFrame* TmpFrame = nullptr;

		while (ret >= 0)
		{
			Frame = av_frame_alloc();
			ret = avcodec_receive_frame(m_Context, Frame);
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) || ret < 0)
			{
				av_frame_free(&Frame);
				break;
			}

			if (Frame->format == m_eHwPixelFormat)
			{
				SwFrame = av_frame_alloc();
				/* Retrieve data from GPU to CPU */
				ret = av_hwframe_transfer_data(SwFrame, Frame, 0);
				if (ret < 0)
				{
					av_frame_free(&Frame);
					av_frame_free(&SwFrame);
					break;
				}

				SwFrame->pts = Frame->pts;
				SwFrame->pkt_dts = Frame->pkt_dts;
				SwFrame->best_effort_timestamp = Frame->best_effort_timestamp;
				TmpFrame = SwFrame;
				av_frame_free(&Frame);
			}
			else
				TmpFrame = Frame;

			if (!n_Func || n_Func(TmpFrame))
			{
				av_frame_free(&TmpFrame);
			}
		}

		return ret;
	}

	int FCodecContext::EncodeFrame(const AVFrame* n_Frame, 
		std::function<int(AVPacket* n_Packet)> n_Func)
	{
		ThrowExceptionExpr(!m_Context, 
			"You should call function Alloc() first.\n");

		int ret = avcodec_send_frame(m_Context, n_Frame);
		if (ret < 0) return ret;
		AVPacket* Packet = nullptr;

		while (ret >= 0)
		{
			Packet = av_packet_alloc();
			ret = avcodec_receive_packet(m_Context, Packet);
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) || ret < 0)
			{
				av_packet_free(&Packet);
				break;
			}

			if (!n_Func || n_Func(Packet))
			{
				av_packet_free(&Packet);
			}
		}

		return ret;
	}


	//////////////////////////////////////////////////////////////////////
	AVEDITOR_API int GetBytesPerSample(AVSampleFormat n_eSampleFormat)
	{
		return av_get_bytes_per_sample(n_eSampleFormat);
	}

	AVEDITOR_API AVCodecContext* CopyCodecContext(
		const AVCodecContext* n_CodecContext)
	{
		AVCodecContext* CodecContext = avcodec_alloc_context3(n_CodecContext->codec);
		if (!CodecContext) return nullptr;

		AVCodecParameters* par = avcodec_parameters_alloc();
		avcodec_parameters_from_context(par, n_CodecContext);

		avcodec_parameters_to_context(CodecContext, par);
		avcodec_parameters_free(&par);

		CodecContext->time_base = n_CodecContext->time_base;
		CodecContext->framerate = n_CodecContext->framerate;

		return CodecContext;
	}
}
