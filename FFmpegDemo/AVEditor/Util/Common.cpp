#include "pch.h"
#include <exception>
#include <iostream>
#include <codecvt>
#include "Common.h"


namespace aveditor 
{
	constexpr size_t BUFFERSIZE_256 = 256;
	constexpr size_t BUFFERSIZE_512 = 512;
	constexpr size_t BUFFERSIZE_2M = 2048;

	char szLogBuffer[BUFFERSIZE_2M] = { 0 };

	void logCallback(void* avcl, int level, const char* fmt, va_list vl)
	{
		memset(szLogBuffer, 0, sizeof(szLogBuffer));
		vsnprintf_s(szLogBuffer, sizeof(szLogBuffer), fmt, vl);

		//printf_s(szLogBuffer);
		std::cout << szLogBuffer;
	}

	AVEDITOR_API void SetupEditorLog()
	{
		av_log_set_callback(logCallback);
	}

	AVEDITOR_API void SetupEditorDevice()
	{
		avdevice_register_all();
	}

	AVEDITOR_API const EStreamType MediaType2StreamType(const AVMediaType n_eMediaType)
	{
		EStreamType eStreamType = EStreamType::ST_Size;

		switch (n_eMediaType)
		{
		case AVMediaType::AVMEDIA_TYPE_VIDEO:
			eStreamType = EStreamType::ST_Video;
			break;
		case AVMediaType::AVMEDIA_TYPE_AUDIO:
			eStreamType = EStreamType::ST_Audio;
			break;
		case AVMediaType::AVMEDIA_TYPE_SUBTITLE:
			eStreamType = EStreamType::ST_Subtitle;
			break;
		}

		return eStreamType;
	}

	AVEDITOR_API const AVMediaType StreamType2MediaType(const EStreamType n_eStreamType)
	{
		AVMediaType eMediaType = AVMediaType::AVMEDIA_TYPE_UNKNOWN;
	
		switch (n_eStreamType)
		{
		case EStreamType::ST_Video:
			eMediaType = AVMediaType::AVMEDIA_TYPE_VIDEO;
			break;
		case EStreamType::ST_Audio:
			eMediaType = AVMediaType::AVMEDIA_TYPE_AUDIO;
			break;
		case EStreamType::ST_Subtitle:
			eMediaType = AVMediaType::AVMEDIA_TYPE_SUBTITLE;
			break;
		}

		return eMediaType;
	}

	AVEDITOR_API void AVFreeData(const EDataType n_eType, void* n_Data)
	{
		if (!n_Data) return;

		switch (n_eType)
		{
		case aveditor::EDataType::DT_None:
			break;
		case aveditor::EDataType::DT_Packet:
		{
			AVPacket* Packet = (AVPacket*)n_Data;
			av_packet_free(&Packet);
		}
			break;
		case aveditor::EDataType::DT_Frame:
		{
			AVFrame* Frame = (AVFrame*)n_Data;
			av_frame_free(&Frame);
		}
			break;
		default:
			break;
		}
	}

	AVEDITOR_API std::string ErrorCode2String(int n_nErrCode)
	{
		std::string sResult;
		sResult.resize(AV_ERROR_MAX_STRING_SIZE);

		av_make_error_string((char*)sResult.c_str(), AV_ERROR_MAX_STRING_SIZE, n_nErrCode);

		return sResult;
	}

	AVEDITOR_API std::string AnsiToUtf8(const std::string& n_sSource)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		std::wstring wstr = conv.from_bytes(n_sSource);
		return conv.to_bytes(wstr);
	}

	AVEDITOR_API std::string StringFormat(const char* n_szFunction, int n_nLine, const char* n_szFormat, ...)
	{
		std::string sResult;
		char szBuffer[BUFFERSIZE_256] = { 0 };
		sprintf_s(szBuffer, BUFFERSIZE_256, "%s [%d]: ", n_szFunction, n_nLine);
		sResult = szBuffer;

		if (n_szFormat && strlen(n_szFormat) > 0)
		{
			va_list args;

			va_start(args, n_szFormat);

			char szContent[BUFFERSIZE_512] = { 0 };
			vsnprintf_s(szContent, BUFFERSIZE_512, n_szFormat, args);
			sResult.append(szContent);

			va_end(args);
		}

		return sResult;
	}

}
