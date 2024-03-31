#include "pch.h"
#include "FormatContext.h"


namespace aveditor
{
	FFormatContext::~FFormatContext()
	{
		Release();
	}

	AVFormatContext* FFormatContext::OpenInputFile(const std::string& n_sFile, 
		const AVInputFormat* n_InputFormat, AVDictionary* n_Options)
	{
		int ret = avformat_open_input(&m_Context, n_sFile.c_str(), 
			n_InputFormat, &n_Options);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to open input file.");

		ret = avformat_find_stream_info(m_Context, nullptr);
		ThrowExceptionCodeExpr(ret < 0, ret, 
			"Fail to find stream info about the input file.");

		return m_Context;
	}

	AVFormatContext* FFormatContext::AllocOutputFile(const std::string& n_sFile, 
		const AVOutputFormat* n_OutputFormat /*= nullptr*/, 
		const char* n_szFormatName /*= nullptr*/)
	{
		std::remove(n_sFile.c_str());

		int ret = avformat_alloc_output_context2(&m_Context, n_OutputFormat, 
			n_szFormatName, n_sFile.c_str());
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to alloc output file.");

		return m_Context;
	}

	void FFormatContext::Release()
	{
		m_mCodecContext.clear();

		if (m_Context)
		{
			if (m_Context->iformat)
				avformat_close_input(&m_Context);
			else
			{
				if (m_Context->pb) avio_close(m_Context->pb);

				avformat_free_context(m_Context);
				m_Context = nullptr;
			}
		}
	}

	const unsigned int FFormatContext::StreamSize()
	{
		if (!m_Context) return 0;

		return m_Context->nb_streams;
	}

	const double FFormatContext::Duration()
	{
		return m_Context ? m_Context->duration * 1.0 / AV_TIME_BASE : 0;
	}

	AVStream* FFormatContext::FindStream(unsigned int n_nStreamIndex)
	{
		if (n_nStreamIndex >= StreamSize()) return nullptr;
		return m_Context->streams[n_nStreamIndex];
	}

	AVStream* FFormatContext::FindStream(AVMediaType n_eMediaType, const AVCodec** n_Codec)
	{
		if (!m_Context) return nullptr;

		int nIndex = av_find_best_stream(m_Context, n_eMediaType, -1, -1, n_Codec, 0);
		return nIndex < 0 ? nullptr : m_Context->streams[nIndex];
	}

	AVStream* FFormatContext::BuildStream(AVCodecContext* n_CodecContext)
	{
		AVStream* Stream = avformat_new_stream(m_Context, nullptr);
		ThrowExceptionExpr(!Stream, "Fail to create stream\n");

		int ret = avcodec_parameters_from_context(Stream->codecpar, n_CodecContext);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to copy parameters from codec context.");

		Stream->codecpar->codec_tag = n_CodecContext->codec_tag;

		return Stream;
	}

	AVStream* FFormatContext::BuildStream(AVStream* n_Stream)
	{
		AVStream* Stream = avformat_new_stream(m_Context, nullptr);
		ThrowExceptionExpr(!Stream, "Fail to create stream\n");

		int ret = avcodec_parameters_copy(Stream->codecpar, n_Stream->codecpar);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to copy parameters from stream.");

		Stream->codecpar->codec_tag = n_Stream->codecpar->codec_tag;

		return Stream;
	}

	AVStream* FFormatContext::BuildStream(FCodecContext& n_CodecContext)
	{
		return BuildStream(n_CodecContext.m_Context);
	}

	std::map<EStreamType, FCodecContext>* FFormatContext::BuildAllStreams(
		FFormatContext& n_InputContext, const int& n_nStream /*= kStreamAll*/)
	{
		ThrowExceptionExpr(!m_Context, "Invalid parameter.\n");

		AVStream* InputStream = nullptr;
		AVStream* OutputStream = nullptr;
		std::map<EStreamType, FCodecContext>* mInputCodecContext = n_InputContext.GetInputCodecContext();

		auto func = 
			[this, &InputStream, &n_InputContext]
			(const AVMediaType n_eMediaType, 
				const AVCodecID n_eCodecID, 
				AVCodecContext* n_CodecContext) {

			InputStream = n_InputContext.FindStream(n_eMediaType);
			if (InputStream &&
				InputStream->codecpar->codec_id == n_eCodecID)
			{
				BuildEncodeCodecContext(InputStream);
			}
			else
			{
				if (n_eCodecID != AVCodecID::AV_CODEC_ID_NONE)
					BuildEncodeCodecContext(n_eCodecID, n_CodecContext);
			}
		};

		for (auto itr = mInputCodecContext->begin(); itr != mInputCodecContext->end(); itr++)
		{
			if ((n_nStream & (1 << (int)itr->first)) == 0) continue;

			switch (itr->first)
			{
			case aveditor::EStreamType::EST_Video:
			{
				func(AVMediaType::AVMEDIA_TYPE_VIDEO,
					m_Context->oformat->video_codec,
					itr->second.m_Context);
			}
				break;
			case aveditor::EStreamType::EST_Audio:
			{
				func(AVMediaType::AVMEDIA_TYPE_AUDIO,
					m_Context->oformat->audio_codec,
					itr->second.m_Context);
			}
				break;
			case aveditor::EStreamType::EST_Subtitle:
			{
				func(AVMediaType::AVMEDIA_TYPE_SUBTITLE,
					m_Context->oformat->subtitle_codec,
					itr->second.m_Context);
			}
				break;
			}

			if (InputStream)
				m_mCodecContext[itr->first].CopyAdditionParameter(InputStream);

			BuildStream(m_mCodecContext[itr->first]);
		}

		return &m_mCodecContext;
	}

	int FFormatContext::ReadPacket(AVPacket* n_Packet)
	{
		int ret = av_read_frame(m_Context, n_Packet);
		ThrowExceptionCodeExpr(ret < 0 && ret != AVERROR_EOF, ret, "Fail to read packet.");

		return ret;
	}

	void FFormatContext::OpenOutputFile()
	{
		ThrowExceptionExpr(!m_Context, 
			"You should alloc call function AllocOutputFile() first.\n");

		int ret = avio_open(&m_Context->pb, m_Context->url, AVIO_FLAG_WRITE);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to open file.");
	}

	void FFormatContext::WriteHeader()
	{
		ThrowExceptionExpr(!m_Context,
			"You should alloc call function AllocOutputFile() first.\n");

		OpenCodecContext();

		for (unsigned int i = 0;i < m_Context->nb_streams; i++)
		{
			AVStream* Stream = m_Context->streams[i];
			EStreamType eStreamType = kStreamIndex.at(Stream->codecpar->codec_type);

			auto itr = m_mCodecContext.find(eStreamType);
			if (itr != m_mCodecContext.end())
			{
				int ret = avcodec_parameters_from_context(
					Stream->codecpar, itr->second.m_Context);
				ThrowExceptionCodeExpr(ret < 0, ret, "Fail to copy parameters from stream.");

				Stream->codecpar->codec_tag = itr->second.m_Context->codec_tag;
			}
		}

		int ret = avformat_write_header(m_Context, nullptr);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to write header into output file.");
	}

	int FFormatContext::InterleavedWritePacket(AVPacket* n_Packet)
	{
		AVRational CodecTimeBase = GetCodecContextTimeBase(n_Packet->stream_index);
		AVRational StreamTimeBase = GetStreamTimeBase(n_Packet->stream_index);
		av_packet_rescale_ts(n_Packet, CodecTimeBase, StreamTimeBase);

		//double dSec = n_Packet->pts * av_q2d(StreamTimeBase);
		//LogInfo("Stream: %d, write packet, pts: %lf.\n", n_Packet->stream_index, dSec);

		int ret = av_interleaved_write_frame(m_Context, n_Packet);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to write packet into output file.");

		return ret;
	}

	int FFormatContext::WritePacket(AVPacket* n_Packet)
	{
		AVRational CodecTimeBase = GetCodecContextTimeBase(n_Packet->stream_index);
		AVRational StreamTimeBase = GetStreamTimeBase(n_Packet->stream_index);
		av_packet_rescale_ts(n_Packet, CodecTimeBase, StreamTimeBase);

		int ret = av_write_frame(m_Context, n_Packet);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to write packet into output file.");

		return ret;
	}

	void FFormatContext::WriteTrailer()
	{
		int ret = av_write_trailer(m_Context);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to write trailer into output file.");
	}

	std::map<EStreamType, FCodecContext>* FFormatContext::GetInputCodecContext()
	{
		if (m_mCodecContext.size() > 0) return GetCodecContext();

		ThrowExceptionExpr(!m_Context || !m_Context->nb_streams, 
			"You should initialize AVFormatContext first.\n");

		AVStream* Stream = nullptr;
		EStreamType eStreamType = EStreamType::EST_Max;

		for (unsigned int i = 0; i < m_Context->nb_streams; i++)
		{
			Stream = m_Context->streams[i];
			if (Stream->codecpar->codec_type != AVMediaType::AVMEDIA_TYPE_UNKNOWN &&
				Stream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_NONE)
			{
				const AVCodec* Codec = FCodecContext::FindDecodeCodec(
					Stream->codecpar->codec_id);

				eStreamType = kStreamIndex.at(Stream->codecpar->codec_type);
				AVCodecContext* ctx = m_mCodecContext[eStreamType].Alloc(Codec);
				m_mCodecContext[eStreamType].CopyCodecParameter(Stream);
				ctx->pkt_timebase = ctx->time_base;
			}
		}

		return &m_mCodecContext;
	}

	std::map<EStreamType, FCodecContext>* FFormatContext::GetOutputCodecContext()
	{
		if (m_mCodecContext.size() > 0) return GetCodecContext();

		ThrowExceptionExpr(!m_Context || !m_Context->nb_streams, 
			"You should initialize AVFormatContext first.\n");

		if (m_Context->oformat->video_codec != AVCodecID::AV_CODEC_ID_NONE)
		{
			BuildEncodeCodecContext(m_Context->oformat->video_codec, nullptr);
		}
		if (m_Context->oformat->audio_codec != AVCodecID::AV_CODEC_ID_NONE)
		{
			BuildEncodeCodecContext(m_Context->oformat->audio_codec, nullptr);
		}
		if (m_Context->oformat->subtitle_codec != AVCodecID::AV_CODEC_ID_NONE)
		{
			BuildEncodeCodecContext(m_Context->oformat->subtitle_codec, nullptr);
		}

		return &m_mCodecContext;
	}

	std::map<EStreamType, FCodecContext>* FFormatContext::GetCodecContext()
	{
		return &m_mCodecContext;
	}

	AVCodecContext* FFormatContext::GetCodecContext(EStreamType n_eStreamType)
	{
		auto itr = m_mCodecContext.find(n_eStreamType);
		if (itr != m_mCodecContext.end())
			return itr->second.m_Context;

		return nullptr;
	}

	FCodecContext* FFormatContext::BuildEncodeCodecContext(AVCodecID n_eCodecID,
		AVCodecContext* n_InputCodecContext)
	{
		const AVCodec* Codec = FCodecContext::FindEncodeCodec(n_eCodecID);

		EStreamType eStreamType = kStreamIndex.at(Codec->type);
		AVCodecContext* ctx = m_mCodecContext[eStreamType].Alloc(Codec);

		if (n_InputCodecContext)
		{
			if (Codec->type == AVMediaType::AVMEDIA_TYPE_AUDIO)
			{
				ctx->bit_rate = n_InputCodecContext->bit_rate;
				ctx->sample_rate = GetSupportedSampleRate(Codec, n_InputCodecContext->sample_rate);
				ctx->sample_fmt = GetSupportedSampleFormat(Codec, n_InputCodecContext->sample_fmt);
				GetSupportedChannelLayout(Codec, &ctx->ch_layout);
				ctx->time_base = { 1, ctx->sample_rate };
			}
			else if (Codec->type == AVMediaType::AVMEDIA_TYPE_VIDEO)
			{
				ctx->bit_rate = n_InputCodecContext->bit_rate;
				ctx->width = n_InputCodecContext->width;
				ctx->height = n_InputCodecContext->height;
				ctx->time_base = n_InputCodecContext->time_base;
				ctx->framerate = av_inv_q(ctx->time_base);
				ctx->pix_fmt = GetSupportedPixelFormat(Codec, n_InputCodecContext->pix_fmt);
			}
		}

		CodecContextAddition(ctx);

		return &m_mCodecContext[eStreamType];
	}

	FCodecContext* FFormatContext::BuildEncodeCodecContext(AVStream* n_Stream)
	{
		const AVCodec* Codec = FCodecContext::FindEncodeCodec(n_Stream->codecpar->codec_id);

		EStreamType eStreamType = kStreamIndex.at(Codec->type);
		AVCodecContext* ctx = m_mCodecContext[eStreamType].Alloc(Codec);

		m_mCodecContext[eStreamType].CopyCodecParameter(n_Stream);
		m_mCodecContext[eStreamType].CopyAdditionParameter(n_Stream);

		CodecContextAddition(ctx);

		return &m_mCodecContext[eStreamType];
	}

	FCodecContext* FFormatContext::BuildDecodeCodecContext(EStreamType n_eStreamType, 
		AVCodecID n_eCodecID)
	{
		auto itr = m_mCodecContext.find(n_eStreamType);
		if (itr != m_mCodecContext.end())
		{
			return &itr->second;
		}

		const AVCodec* Codec = FCodecContext::FindDecodeCodec(n_eCodecID);
		m_mCodecContext[n_eStreamType].Alloc(Codec);

		return &m_mCodecContext[n_eStreamType];
	}

	bool FFormatContext::IsSupportBFrame()
	{
		if (!m_Context) return false;

		auto itr = m_mCodecContext.find(EStreamType::EST_Video);
		if (itr == m_mCodecContext.end()) return false;

		return itr->second.m_Context->codec->capabilities & AV_CODEC_CAP_DELAY;
	}

	void FFormatContext::OpenCodecContext(const EStreamType n_eStreamType)
	{
		for (auto itr = m_mCodecContext.begin(); itr != m_mCodecContext.end(); itr++)
		{
			if (n_eStreamType == EStreamType::EST_Max || n_eStreamType == itr->first)
			{
				itr->second.Open();
			}
		}
	}

	void FFormatContext::CloseCodecContext(const EStreamType n_eStreamType)
	{
		for (auto itr = m_mCodecContext.begin(); itr != m_mCodecContext.end(); itr++)
		{
			if (n_eStreamType == EStreamType::EST_Max || n_eStreamType == itr->first)
			{
				itr->second.Close();
			}
		}
	}

	AVRational FFormatContext::GetCodecContextTimeBase(unsigned int n_nStreamIndex)
	{
		AVStream* Stream = FindStream(n_nStreamIndex);

		if (Stream)
		{
			EStreamType eStreamType = kStreamIndex.at(Stream->codecpar->codec_type);
			auto itr = m_mCodecContext.find(eStreamType);
			if (itr != m_mCodecContext.end())
				return itr->second.m_Context->time_base;
		}

		return AVRational{ 0, 1 };
	}

	AVRational FFormatContext::GetCodecContextTimeBase(const EStreamType n_eStreamType)
	{
		auto itr = m_mCodecContext.find(n_eStreamType);
		if (itr != m_mCodecContext.end())
			return itr->second.m_Context->time_base;

		return AVRational{ 0, 1 };
	}

	AVRational FFormatContext::GetStreamTimeBase(unsigned int n_nStreamIndex)
	{
		if (n_nStreamIndex < m_Context->nb_streams)
			return m_Context->streams[n_nStreamIndex]->time_base;

		return AVRational{ 0, 1 };
	}

	void FFormatContext::CodecContextAddition(AVCodecContext* n_CodecContext)
	{
		/* Some formats want stream headers to be separate. */
		/* Some container formats (like MP4) require global headers to be present.
		 * Mark the encoder so that it behaves accordingly. */
		
		//if (m_Context->oformat->flags & AVFMT_GLOBALHEADER)
		//{
		//	n_CodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		//	unsigned char sps_pps[23] = { 0x00, 0x00, 0x00, 0x01, 0x67 };
		//	n_CodecContext->extradata_size = 23;
		//	n_CodecContext->extradata = (uint8_t*)av_malloc(23 + AV_INPUT_BUFFER_PADDING_SIZE);
		//	memcpy_s(n_CodecContext->extradata, 23, sps_pps, 23);
		//}

		if (n_CodecContext->codec_id == AVCodecID::AV_CODEC_ID_AAC)
		{
			if (n_CodecContext->bit_rate < 100 || n_CodecContext->bit_rate > 500000)
				n_CodecContext->bit_rate = 128000;
		}
		else if (n_CodecContext->codec_id == AVCodecID::AV_CODEC_ID_MPEG4)
		{
			if (n_CodecContext->time_base.den > 65535)
				n_CodecContext->time_base.den = 65535;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AVEDITOR_API AVSampleFormat GetSupportedSampleFormat(const AVCodec* n_Codec, 
		enum AVSampleFormat n_eSampleFormat)
	{
		if (!n_Codec || !n_Codec->sample_fmts || 
			AVSampleFormat::AV_SAMPLE_FMT_NONE == *n_Codec->sample_fmts)
			return n_eSampleFormat;

		const enum AVSampleFormat* p = n_Codec->sample_fmts;

		AVSampleFormat SampleFormat = *p;

		while (p && *p != AVSampleFormat::AV_SAMPLE_FMT_NONE)
		{
			if (*p == n_eSampleFormat)
			{
				SampleFormat = *p;
				break;
			}

			p++;
		}

		return SampleFormat;
	}

	AVEDITOR_API int GetSupportedSampleRate(const AVCodec* n_Codec, int n_nSampleRate)
	{
		if (!n_Codec || 
			!n_Codec->supported_samplerates || 
			0 == *n_Codec->supported_samplerates)
			return n_nSampleRate;

		const int* p = n_Codec->supported_samplerates;

		int nSampleRate = *p;
		int nDiff = abs(n_nSampleRate - *p);
		p++;

		while (p && *p)
		{
			int v = abs(n_nSampleRate - *p);
			if (nDiff > v)
			{
				nDiff = v;
				nSampleRate = *p;
			}

			p++;
		}

		return nSampleRate;
	}

	AVEDITOR_API int GetSupportedChannelLayout(const AVCodec* n_Codec, 
		AVChannelLayout* n_ChannelLayout)
	{
		if (!n_Codec || !n_Codec->ch_layouts)
		{
			AVChannelLayout ChannelLayout;
			ChannelLayout.order = AV_CHANNEL_ORDER_NATIVE;
			ChannelLayout.nb_channels = 2;
			ChannelLayout.u.mask = AV_CH_LAYOUT_STEREO;

			return av_channel_layout_copy(n_ChannelLayout, &ChannelLayout);
		}

		const AVChannelLayout* p = n_Codec->ch_layouts;
		const AVChannelLayout* BestChannelLayout = p;
		int nBestNbChannels = 0;

		while (p && p->nb_channels)
		{
			if (p->nb_channels > nBestNbChannels)
			{
				nBestNbChannels = p->nb_channels;
				BestChannelLayout = p;
			}

			p++;
		}

		return av_channel_layout_copy(n_ChannelLayout, BestChannelLayout);
	}

	AVEDITOR_API AVPixelFormat GetSupportedPixelFormat(const AVCodec* n_Codec, 
		AVPixelFormat n_ePixelFormat)
	{
		if (!n_Codec || !n_Codec->pix_fmts ||
			AVPixelFormat::AV_PIX_FMT_NONE == *n_Codec->pix_fmts)
			return n_ePixelFormat;

		const AVPixelFormat* p = n_Codec->pix_fmts;
		AVPixelFormat PixelFormat = *p;

		while (p && *p != AVPixelFormat::AV_PIX_FMT_NONE)
		{
			if (*p == n_ePixelFormat)
			{
				PixelFormat = *p;
				break;
			}

			p++;
		}

		return PixelFormat;
	}

	AVEDITOR_API AVRational GetSupportedFrameRate(const AVCodec* n_Codec, 
		const AVRational& n_FrameRate)
	{
		if (!n_Codec || !n_Codec->supported_framerates)
			return n_FrameRate;

		const AVRational* p = n_Codec->supported_framerates;

		if (!p || p->num == 0 || p->den == 0) return n_FrameRate;

		AVRational Rational = *p++;
		double dBase = av_q2d(n_FrameRate);
		double dCurrent = av_q2d(Rational);
		double dDiff = abs(dCurrent - dBase);

		while (p)
		{
			if (p->num)
			{
				dCurrent = av_q2d(*p);
				double v = abs(dCurrent - dBase);

				if (dDiff > v)
				{
					dDiff = v;
					Rational = *p;
				}
			}

			p++;
		}

		return Rational;
	}

	AVEDITOR_API int FindStreamIndex(AVFormatContext* n_FormatContext, AVMediaType n_eMediaType)
	{
		return av_find_best_stream(n_FormatContext, n_eMediaType, -1, -1, nullptr, 0);
	}

	AVEDITOR_API const AVInputFormat* FindInputFormat(const std::string& n_sShortName)
	{
		return av_find_input_format(n_sShortName.c_str());
	}

	AVEDITOR_API std::map<int, AVStream*> FindAllStream(AVFormatContext* n_FormatContext)
	{
		std::map<int, AVStream*> mResult;

		if (!n_FormatContext) return mResult;

		AVStream* Stream = nullptr;

		for (unsigned int i = 0; i < n_FormatContext->nb_streams; i++)
		{
			Stream = n_FormatContext->streams[i];
			if (Stream->codecpar->codec_type != AVMediaType::AVMEDIA_TYPE_UNKNOWN &&
				Stream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_NONE)
			{
				mResult.insert({ i, Stream });
			}
		}

		return mResult;
	}

}
