#ifndef __LATHEPARTS_H__
#define __LATHEPARTS_H__
#include "Apis/CodecContext.h"
#include "Apis/AudioFifo.h"
#include "Apis/Resample.h"
#include "Apis/SwsScale.h"
#include "Filter/Filter.h"
#include "Util/Queue.h"
#include "Core/DataItem.h"


namespace avstudio
{
	struct FLatheParts
	{
		// Indicate if it should be decoded
		int				nShouldDecode = 0;
		// Index of video stream, -1: invalid
		int				nStreamIndex = -1;
		// For output context, means weather current input context is done
		// For input context, not use
		bool			bIsEnd = true;

		/*
		* For output context only
		* If AVStudio create codec context by default automatic,
		* it will choose [DesireCodecId] if it's valid
		* Notice: it may be incompatible with output context
		*/
		AVCodecID		DesireCodecId = AVCodecID::AV_CODEC_ID_NONE;

		// Frame duration, for output context only
		int64_t			Duration = 1;
		// For output context only, if output context is valid, 
		// it will be initialize
		Queue<void*>*	Buffer = nullptr;

		AVStream*		Stream = nullptr;
		// Video codec
		FCodecContext*	Codec = nullptr;
		// Point to Filter instance
		IFilter*		Filter = nullptr;
		FSwsScale*		Sws = nullptr;
		FResample*		Resample = nullptr;
		FAudioFifo*		FiFo = nullptr;

		void Release();
		void CleanBuffer();
		void ReleaseBuffer();

		AVCodecID CodecID() const;
	};

}


#endif // !__LATHEPARTS_H__
