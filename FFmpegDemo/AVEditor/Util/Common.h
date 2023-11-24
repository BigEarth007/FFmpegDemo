#pragma once

namespace aveditor 
{
	// What to do with the input context
	enum class EJob
	{
		// merge, cut, cover..
		EJ_Normal = 0,
		// will mix other audio
		EJ_AMixMain,
		// will mix to other audio
		EJ_AMixBranch,
	};

	enum class EStreamType
	{
		// Video Stream
		EST_Video = 0,
		// Audio Stream
		EST_Audio,
		// Subtitle Stream
		EST_Subtitle,
		EST_Max
	};

	constexpr auto kStreamVideo = (1 << (int)EStreamType::EST_Video);
	constexpr auto kStreamAudio = (1 << (int)EStreamType::EST_Audio);
	constexpr auto kStreamSubtitle = (1 << (int)EStreamType::EST_Subtitle);
	constexpr auto kStreamVA = kStreamAudio | kStreamVideo;
	constexpr auto kStreamAll = kStreamVA | kStreamSubtitle;

	constexpr auto kSleepDelay = 2;			// millisecond
	constexpr auto kSleepTimeout = 10;		// millisecond the max time when pop queue data
	constexpr auto kEditorFactor = 100;
	constexpr auto kEditorIndexFactor = 10;

	enum class AVEDITOR_API EStage
	{
		// 解复用
		ES_Demux = 1,
		// 解码
		ES_Decode,
		// 过滤器
		ES_Filter,
		// 编码
		ES_Encode,
		// 复用
		ES_Mux,
	};

	// Match the stream type and stream
	const std::map<AVMediaType, EStreamType> kStreamIndex =
	{
		std::pair<AVMediaType, EStreamType>(
			AVMediaType::AVMEDIA_TYPE_VIDEO, EStreamType::EST_Video),
		std::pair<AVMediaType, EStreamType>(
			AVMediaType::AVMEDIA_TYPE_AUDIO, EStreamType::EST_Audio),
		std::pair<AVMediaType, EStreamType>(
			AVMediaType::AVMEDIA_TYPE_SUBTITLE, EStreamType::EST_Subtitle)
	};

	extern "C"
	{
		// set up log
		AVEDITOR_API void SetupEditorLog();
		// register all device
		AVEDITOR_API void SetupEditorDevice();
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
