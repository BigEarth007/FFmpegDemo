#pragma once


namespace aveditor
{
	struct AVEDITOR_API FCodecContext
	{
		FCodecContext() = default;
		~FCodecContext();

		// Find decode codec by id
		static const AVCodec* FindDecodeCodec(AVCodecID n_CodecID);
		// Find decode codec by name
		static const AVCodec* FindDecodeCodec(const char* n_szName);
		// Find encode codec by id
		static const AVCodec* FindEncodeCodec(AVCodecID n_CodecID);
		// Find encode codec by name
		static const AVCodec* FindEncodeCodec(const char* n_szName);
		// Check if hardware encode supported
		static bool IsHwEncodeSupported();

		// Alloc codec context memory
		AVCodecContext* Alloc(const AVCodec* n_Codec);
		// Release 
		void Release();

		// Get pixel format for hardware codec
		int GetHwPixelFormat(const AVCodec* n_Codec, AVHWDeviceType n_eHwDeviceType,
			AVPixelFormat& n_ePixelFormat);

		// Open a device of the specified type and create an AVHWDeviceContext
		void HwInit(AVHWDeviceType n_eHwDeviceType);

		// Copy codec parameter form stream
		void CopyCodecParameter(const AVStream* n_Stream);
		void CopyCodecParameter(const AVCodecContext* n_CodecContext);
		void CopyAdditionParameter(const AVStream* n_Stream);
		// Open codec context
		void Open(AVDictionary** n_Options = nullptr);
		// Close codec context
		void Close();

		// Decode AVPacket
		// n_Func return value: weather free n_Frame or not
		//		1: auto free n_Frame
		//		0: no need to free n_Frame
		int DecodePacket(const AVPacket* n_Packet, 
			std::function<int(AVFrame* n_Frame)> n_Func);
		// Encode AVFrame
		// n_Func return value: weather free n_Packet or not
		//		1: auto free n_Packet
		//		0: no need to free n_Packet
		int EncodeFrame(const AVFrame* n_Frame,
			std::function<int(AVPacket* n_Packet)> n_Func);

		// Video: Get number of planes by pixel format
		int GetPixFmtPlaneCount();

		// Audio: Check if the sample format is planar.
		int IsSampleFmtPlanar();

		// Audio: number of bytes per sample
		int GetBytesPerSample();

		AVCodecContext* m_Context = nullptr;

	protected:
		AVPixelFormat	m_eHwPixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;
		AVBufferRef*	m_HwDeviceContext = nullptr;
	};


	//////////////////////////////////////////////////////////////////////
	extern "C"
	{
		// Video: Get number of planes by pixel format
		AVEDITOR_API int GetPixFmtPlaneCount(AVPixelFormat n_ePixelFormat);

		// Audio: Check if the sample format is planar.
		AVEDITOR_API int IsSampleFmtPlanar(AVSampleFormat n_eSampleFormat);

		// Audio: number of bytes per sample
		AVEDITOR_API int GetBytesPerSample(AVSampleFormat n_eSampleFormat);

		AVEDITOR_API AVCodecContext* CopyCodecContext(
			const AVCodecContext* n_CodecContext);
	}
}
