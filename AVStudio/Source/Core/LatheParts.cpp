#include "Core/LatheParts.h"



namespace avstudio
{
	void FLatheParts::Release()
	{
		nFlag = 0;
		nStreamIndex = -1;
		Duration = 0;
		PacketPts = 0;
		FramePts = 0;

		Stream = nullptr;

		// If reference count lager than 0, it will not call Release()
		if (Codec) Codec->Release();
		Codec.reset();
		Sws.reset();
		Resample.reset();
		FiFo.reset();
		Filter.reset();

		DesireCodecId = AVCodecID::AV_CODEC_ID_NONE;
	}

	AVCodecID FLatheParts::CodecID() const
	{
		AVCodecID Result = AVCodecID::AV_CODEC_ID_NONE;

		if (Codec)
			Result = Codec->Context->codec_id;
		else if (Stream)
			Result = Stream->codecpar->codec_id;

		return Result;
	}

}
