#include "pch.h"
#include "Muxer.h"


namespace aveditor
{
	CMuxer::CMuxer(FCache& n_Cache, const int& n_nPrefix)
		: IStage(n_Cache, n_nPrefix, EStreamType::EST_Max)
	{
	}

	CMuxer::~CMuxer()
	{
	}

	void CMuxer::Init(FFormatContext& n_OutputContext)
	{
		m_OutputContext = &n_OutputContext;
	}

	void CMuxer::Run()
	{
		ThrowExceptionExpr(!m_OutputContext,
			"You should call function Init() first.\n");
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		EStage eStage = EStage::ES_Encode;
		if (!m_Cache->IsStageExist(eStage))
			eStage = EStage::ES_Demux;

		try
		{
			auto& vContextInfo = m_Cache->GetContextInfos();
			for (size_t i = 0; i < vContextInfo.size(); i++)
			{
				if (vContextInfo[i].eJob == EJob::EJ_AMixBranch) continue;

				if (vContextInfo[i].nStreams[(int)EStreamType::EST_Video] > -1)
				{
					AVStream* Stream = m_OutputContext->FindStream(
						AVMediaType::AVMEDIA_TYPE_VIDEO);
					ThrowExceptionExpr(!Stream, 
						"Missing video stream for output context.\n");
				}

				m_nPreviousPrefix = StageToPrefix(eStage, (int)i);
				m_Cache->SetSelectedContextIndex((int)i);
				Muxing(vContextInfo[i]);
			}
		}
		catch (std::exception& e)
		{
			av_log(nullptr, AV_LOG_INFO, e.what());
			av_log(nullptr, AV_LOG_INFO, "\n");
		}

		Thread::Run();
	}

	void CMuxer::Muxing(const FContextInfo& n_ContextInfo)
	{
		int			ret = 0;
		// Current key of current stream index
		int			nCurKey = 0;
		// Number of stream type that have finished
		int			nCount = 0;
		// The stream type which got the packet with min timestamp
		int			nMinStreamType = 0;
		// Cast duration of last input context to pts
		int64_t		nPts = 0;

		FPacketCache mPacketCache[(int)EStreamType::EST_Max];

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			AVRational Timebase =
				m_OutputContext->GetCodecContextTimeBase((EStreamType)i);
			mPacketCache[i].dTimebase = av_q2d(Timebase);

			nCurKey = m_nPreviousPrefix + i;
			mPacketCache[i].QueueItem = m_Cache->GetBufferQueue(nCurKey);

			if (n_ContextInfo.nStreams[i] == -1)
				mPacketCache[i].bFinished = true;
		}

		while (!IsStop())
		{
			for (int i = 0; i < (int)EStreamType::EST_Max; i++)
			{
				if (mPacketCache[i].Packet || mPacketCache[i].bFinished)
					continue;

				// Get the first packet
				ret = m_Cache->Pop(mPacketCache[i].QueueItem, 
					mPacketCache[i].Packet);
				if (ret < 0) break;

				// If Packet is nullptr, it's last packet
				if (!mPacketCache[i].Packet)
				{
					mPacketCache[i].bFinished = true;
					continue;
				}

				mPacketCache[i].dTimestamp =
					mPacketCache[i].Packet->pts * mPacketCache[i].dTimebase;
			}

			// One or more streams are waiting for packet
			if (ret < 0)
			{
				Sleep(kSleepDelay);
				continue;
			}

			nCount = 0;
			nMinStreamType = -1;

			for (int i = 0; i < (int)EStreamType::EST_Max; i++)
			{
				if (mPacketCache[i].bFinished)
					nCount++;
				else if (nMinStreamType == -1 ||
					mPacketCache[nMinStreamType].dTimestamp > mPacketCache[i].dTimestamp)
					nMinStreamType = i;
			}

			// All stream queues have pop all packet
			if ((int)EStreamType::EST_Max == nCount) break;
			// The duration of audio stream should not large than video stream
			if (mPacketCache[(int)EStreamType::EST_Video].bFinished &&
				mPacketCache[(int)EStreamType::EST_Video].dTimestamp < 
					mPacketCache[nMinStreamType].dTimestamp)
				break;

			if (mPacketCache[nMinStreamType].Packet->pts < 0)
			{
				m_dDuration -= mPacketCache[nMinStreamType].dTimestamp;
				nPts -= mPacketCache[nMinStreamType].Packet->pts;
			}

			nPts = (int64_t)(m_dDuration / mPacketCache[nMinStreamType].dTimebase);
			mPacketCache[nMinStreamType].Packet->stream_index = 
				n_ContextInfo.nStreams[nMinStreamType];
			mPacketCache[nMinStreamType].Packet->pts += nPts;
			mPacketCache[nMinStreamType].Packet->dts += nPts;

			m_OutputContext->InterleavedWritePacket(mPacketCache[nMinStreamType].Packet);

			av_packet_free(&mPacketCache[nMinStreamType].Packet);
			mPacketCache[nMinStreamType].Packet = nullptr;

			//m_Cache->DebugBufferQueue();
		}

		m_dDuration += n_ContextInfo.dDuration;
	}

}
