#pragma once

namespace aveditor 
{
	enum class AVEDITOR_API EDataType
	{
		DT_None = 0,
		DT_Packet,
		DT_Frame,
	};

	// What to do with the input context
	enum class AVEDITOR_API ETask
	{
		// merge, cut, cover..
		T_Normal = 0,
		// will mix other audio
		T_AMixMain,
		// will be mixed to other audio
		T_AMixBranch,
	};

	enum class AVEDITOR_API EStreamType
	{
		// Video Stream
		ST_Video = 0,
		// Audio Stream
		ST_Audio,
		// Subtitle Stream
		ST_Subtitle,
		ST_Size
	};

	constexpr auto kStreamVideo = (1 << (int)EStreamType::ST_Video);
	constexpr auto kStreamAudio = (1 << (int)EStreamType::ST_Audio);
	constexpr auto kStreamSubtitle = (1 << (int)EStreamType::ST_Subtitle);
	constexpr auto kStreamVA = kStreamAudio | kStreamVideo;
	constexpr auto kStreamAll = kStreamVA | kStreamSubtitle;

	constexpr auto kSleepDelay = 2;			// millisecond
	constexpr auto kSleepTimeout = 10;		// millisecond the max time when pop queue data
	constexpr auto kEditorFactor = 100;
	constexpr auto kEditorIndexFactor = 10;


	enum class AVEDITOR_API ECompID
	{
		EI_Demux = 0,
		EI_Decode,
		EI_Filter,
		EI_Encode,
		EI_Muxer,
	};

	extern "C"
	{
		// set up log
		AVEDITOR_API void SetupEditorLog();
		// register all device
		AVEDITOR_API void SetupEditorDevice();
		// Cover media type to stream type
		AVEDITOR_API const EStreamType MediaType2StreamType(const AVMediaType n_eMediaType);
		// Free AVPacket/AVFrame by DataType
		AVEDITOR_API void AVFreeData(const EDataType n_eType, void* n_Data);
	}

	AVEDITOR_API std::string ErrorCode2String(int n_nErrCode);

	AVEDITOR_API std::string AnsiToUtf8(const std::string& n_sSource);

	// format string with function name and line
	AVEDITOR_API std::string StringFormat(const char* n_szFunction, int n_nLine, const char* n_szFormat, ...);

	// if expr is true, throw std::exception with function name and line
#define ThrowExceptionExpr(expr, fmt, ...) \
	if (expr) { \
		std::string sException = StringFormat(__FUNCTION__, __LINE__, fmt, __VA_ARGS__); \
		throw std::exception(sException.c_str()); \
	}

	// if expr is true, throw std::exception with function name and line
#define ThrowExceptionCodeExpr(expr, err_code, fmt, ...) \
	if (expr) { \
		std::string sErrMessage = ErrorCode2String(err_code); \
		std::string sException = StringFormat(__FUNCTION__, __LINE__, fmt, __VA_ARGS__); \
		sException.append("  Err: ").append(sErrMessage).append("\n"); \
		throw std::exception(sException.c_str()); \
	}

#define LogInfo(fmt, ...) \
	{ \
		std::string sFormat = StringFormat(__FUNCTION__, __LINE__, nullptr); \
		sFormat.append(fmt); \
		av_log(nullptr, AV_LOG_INFO, sFormat.c_str(), __VA_ARGS__); \
	}

#define LogInfoExpr(expr, fmt, ...) \
	if (expr) { \
		std::string sFormat = StringFormat(__FUNCTION__, __LINE__, nullptr); \
		sFormat.append(fmt); \
		av_log(nullptr, AV_LOG_INFO, sFormat.c_str(), __VA_ARGS__); \
	}

	// debug log info
#ifdef _DEBUG
#define DebugLog(fmt, ...) LogInfo(fmt, __VA_ARGS__)
#else
#define DebugLog(fmt, ...)
#endif // DEBUG

	template<typename T, typename R>
	void ReleaseMap(std::map<T, R*>& n_mObj)
	{
		for (auto itr = n_mObj.begin();itr != n_mObj.end(); itr++)
		{
			delete itr->second;
			itr->second = nullptr;
		}

		n_mObj.clear();
	}

	template<typename T>
	void ReleaseVector(std::vector<T*>& n_vObj)
	{
		for (size_t i = 0; i < n_vObj.size(); i++)
		{
			delete n_vObj[i];
			n_vObj[i] = nullptr;
		}

		n_vObj.clear();
	}
}
