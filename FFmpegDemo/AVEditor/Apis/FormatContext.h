#pragma once


namespace aveditor
{
	struct AVEDITOR_API FFormatContext
	{
		FFormatContext() = default;
		~FFormatContext();

		// Open the input file
		AVFormatContext* OpenInputFile(const std::string& n_sFile, 
			const AVInputFormat* n_InputFormat = nullptr, 
			AVDictionary* n_Options = nullptr);
		// Alloc output file memory
		AVFormatContext* AllocOutputFile(const std::string& n_sFile,
			const AVOutputFormat* n_OutputFormat = nullptr,
			const char* n_szFormatName = nullptr);
		// Release format context
		void Release();

		// Get the count of stream
		const unsigned int StreamSize();
		// The length of the context
		const double Duration();

		// Get stream
		AVStream* FindStream(unsigned int n_nStreamIndex);
		// Get stream by media type: n_eMediaType
		AVStream* FindStream(AVMediaType n_eMediaType, const AVCodec** n_Codec = nullptr);
		// Create stream for output file
		AVStream* BuildStream(AVCodecContext* n_CodecContext);
		AVStream* BuildStream(AVStream* n_Stream);
		AVStream* BuildStream(FCodecContext& n_CodecContext);
		std::map<EStreamType, FCodecContext>* BuildAllStreams(
			FFormatContext& n_InputContext, const int& n_nStream = kStreamAll);

		// Read packet from input file
		int ReadPacket(AVPacket* n_Packet);

		// open the output file
		void OpenOutputFile();

		// Write header into output file
		void WriteHeader();
		// Write packet into output file
		int InterleavedWritePacket(AVPacket* n_Packet);
		int WritePacket(AVPacket* n_Packet);
		// Write trailer into output file
		void WriteTrailer();

		// Get codec context from input AVFormatContext
		std::map<EStreamType, FCodecContext>* GetInputCodecContext();
		// Get codec context from output AVFormatContext
		std::map<EStreamType, FCodecContext>* GetOutputCodecContext();
		// Get codec context
		std::map<EStreamType, FCodecContext>* GetCodecContext();
		AVCodecContext* GetCodecContext(EStreamType n_eStreamType);

		// Build encode codec context
		FCodecContext* BuildCodecContext(AVCodecID n_eCodecID, 
			AVCodecContext* n_InputCodecContext);
		FCodecContext* BuildCodecContext(AVStream* n_Stream);
		// Check if supports B-Frame
		bool IsSupportBFrame();
		// Open codec context; 
		// If n_eStreamType == EStreamType::EST_Max, open all the codec context
		void OpenCodecContext(const EStreamType n_eStreamType = EStreamType::EST_Max);
		// Close codec context; 
		// If n_eStreamType == EStreamType::EST_Max, close all the codec context
		void CloseCodecContext(const EStreamType n_eStreamType = EStreamType::EST_Max);

		AVRational GetCodecContextTimeBase(unsigned int n_nStreamIndex);
		AVRational GetCodecContextTimeBase(const EStreamType n_eStreamType);
		AVRational GetStreamTimeBase(unsigned int n_nStreamIndex);

		AVFormatContext*						m_Context = nullptr;

		/* Codec Context of every stream */ 
		std::map<EStreamType, FCodecContext>	m_mCodecContext;

	protected:
		// Addition setting for codec context
		void CodecContextAddition(AVCodecContext* n_CodecContext);
	};


	extern "C"
	{
		// Get the sample format supported by n_Codec, the n_eSampleFormat comes first
		AVEDITOR_API AVSampleFormat GetSupportedSampleFormat(const AVCodec* n_Codec, 
			enum AVSampleFormat n_eSampleFormat);
		// Get the sample rate supported by n_Codec, select the most closest to n_nSampleRate
		AVEDITOR_API int GetSupportedSampleRate(const AVCodec* n_Codec, int n_nSampleRate);
		// Get layout with the highest channel count
		AVEDITOR_API int GetSupportedChannelLayout(const AVCodec* n_Codec, 
			AVChannelLayout* n_ChannelLayout);

		// Get pixel format supported by n_Codec, the n_ePixelFormat comes first
		AVEDITOR_API AVPixelFormat GetSupportedPixelFormat(const AVCodec* n_Codec, 
			AVPixelFormat n_ePixelFormat);
		// Get the frame rate supported by n_Codec, select the most closest to n_FrameRate
		AVEDITOR_API AVRational GetSupportedFrameRate(const AVCodec* n_Codec, 
			const AVRational& n_FrameRate);

		AVEDITOR_API int FindStreamIndex(AVFormatContext* n_FormatContext, AVMediaType n_eMediaType);

		AVEDITOR_API const AVInputFormat* FindInputFormat(const std::string& n_sShortName);

	}

	// Get all streams, just support video, audio, subtitle
	AVEDITOR_API std::map<int, AVStream*> FindAllStream(AVFormatContext* n_FormatContext);
}
