#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif

#define AVEDITOR_API
//#ifdef AVEDITOR_EXPORTS
//#define AVEDITOR_API __declspec(dllexport)
//#else
//#define AVEDITOR_API __declspec(dllimport)
//#endif

#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")

#include "Util/Common.h"
#include "Util/Thread.h"
#include "Util/Queue.h"
#include "Util/AVQueue.h"

#include "Apis/CodecContext.h"
#include "Apis/FormatContext.h"
#include "Apis/Packet.h"
#include "Apis/Frame.h"
#include "Apis/Cale.h"
#include "Apis/Resample.h"
#include "Apis/AudioFifo.h"
#include "Apis/Device.h"
#include "Apis/Filter.h"

#include "Component/AVIOHandle.h"
#include "Component/Component.h"
#include "Component/DemuxComponent.h"
#include "Component/DecodeComponent.h"
#include "Component/AudioMixComponent.h"
#include "Component/EncodeComponent.h"
#include "Component/MuxerComponent.h"
#include "Component/AVObject.h"

#include "Core/Stage.h"
#include "Core/Filtrate.h"
#include "Core/Convert.h"
#include "Core/Demuxer.h"
#include "Core/Muxer.h"
#include "Core/Encoder.h"
#include "Core/Decoder.h"

#include "Editor/ContextHandle.h"
#include "Editor/BaseContext.h"
#include "Editor/InputContext.h"
#include "Editor/OutputContext.h"
#include "Editor/Editor.h"
