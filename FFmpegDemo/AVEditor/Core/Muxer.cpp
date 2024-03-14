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

				m_Cache->SetSelectedContextIndex((int)i);
				Muxing((int)i, vContextInfo[i]);
			}
		}
		catch (std::exception& e)
		{
			av_log(nullptr, AV_LOG_INFO, e.what());
			av_log(nullptr, AV_LOG_INFO, "\n");
		}

		Thread::Run();
	}

	void CMuxer::Muxing(const int n_nIndex, const FContextInfo& n_ContextInfo)
	{
		int			ret = 0;
		// Current key of current stream index
		int			nKey = 0;
		// Number of stream type that have finished
		int			nCount = 0;
		// The stream type which got the packet with min timestamp
		int			nMinStreamType = 0;
		// Cast duration of last input context to pts
		int64_t		nPts = 0;

		FPacketCache PacketCaches[(int)EStreamType::EST_Max];

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (n_ContextInfo.nStreams[i] == -1) continue;
			PacketCaches[i].nStatus = 0;

			AVRational Timebase =
				m_OutputContext->GetCodecContextTimeBase((EStreamType)i);
			PacketCaches[i].dTimebase = av_q2d(Timebase);

			nKey = m_nCurrentPrefix + n_nIndex * kEditorIndexFactor + i;
			nKey = m_Cache->GetPreviousKeyPrefix(nKey) + i;
			PacketCaches[i].QueueItem = m_Cache->GetBufferQueue(nKey);
		}

		while (!IsStop())
		{
			for (int i = 0; i < (int)EStreamType::EST_Max; i++)
			{
				if (PacketCaches[i].Packet || PacketCaches[i].nStatus != 0)
					continue;

				// Get the first packet
				ret = m_Cache->Pop(PacketCaches[i].QueueItem, 
					PacketCaches[i].Packet);
				if (ret < 0) break;

				// If Packet is nullptr, it's last packet
				if (!PacketCaches[i].Packet)
				{
					PacketCaches[i].nStatus = 1;
					continue;
				}

				PacketCaches[i].dTimestamp =
					PacketCaches[i].Packet->pts * PacketCaches[i].dTimebase;
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
				if (PacketCaches[i].nStatus != 0)
					nCount++;
				else if (nMinStreamType == -1 ||
					PacketCaches[nMinStreamType].dTimestamp > PacketCaches[i].dTimestamp)
					nMinStreamType = i;
			}

			// All stream queues have pop all packet
			if ((int)EStreamType::EST_Max == nCount) break;
			// The duration of audio stream should not large than video stream
			if (PacketCaches[(int)EStreamType::EST_Video].nStatus == 1 &&
				PacketCaches[(int)EStreamType::EST_Video].dTimestamp < 
					PacketCaches[nMinStreamType].dTimestamp)
				break;

			if (PacketCaches[nMinStreamType].Packet->pts < 0)
			{
				m_dDuration -= PacketCaches[nMinStreamType].dTimestamp;
				nPts -= PacketCaches[nMinStreamType].Packet->pts;
			}

			nPts = (int64_t)(m_dDuration / PacketCaches[nMinStreamType].dTimebase);
			PacketCaches[nMinStreamType].Packet->stream_index = 
				n_ContextInfo.nStreams[nMinStreamType];
			PacketCaches[nMinStreamType].Packet->pts += nPts;
			PacketCaches[nMinStreamType].Packet->dts += nPts;

			m_OutputContext->InterleavedWritePacket(PacketCaches[nMinStreamType].Packet);

			av_packet_free(&PacketCaches[nMinStreamType].Packet);
			PacketCaches[nMinStreamType].Packet = nullptr;

			//m_Cache->DebugBufferQueue();
		}

		m_dDuration += n_ContextInfo.dDuration;
	}

}
