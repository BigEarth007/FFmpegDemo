#include "pch.h"
#include "Filtrate.h"


namespace aveditor
{
	CFiltrate::CFiltrate(FCache& n_Cache, const int& n_nPrefix,
		const EStreamType& n_eStreamType)
		: IStage(n_Cache, n_nPrefix, n_eStreamType)
	{
	}

	CFiltrate::~CFiltrate()
	{
		Release();
	}

	void CFiltrate::Init(FCodecContext& n_OutputCodecContext)
	{
		CreateMixFilter(n_OutputCodecContext.m_Context);
		m_OutputCodecContext = n_OutputCodecContext.m_Context;
	}

	void CFiltrate::Run()
	{
		ThrowExceptionExpr(!m_Cache, "Invalid buffer.\n");

		if (!m_Filter.m_FilterGraph) return;

		int			ret = 0;
		int			nKeyPrevious = m_nPreviousPrefix + (int)m_eStreamType;
		int			nKeyCurrent = m_nCurrentPrefix + (int)m_eStreamType;
		AVFrame*	Frame = nullptr;

		std::vector<FContextInfo>	vContextInfos = m_Cache->GetContextInfos();
		std::vector<CQueueItem*>	vQueueItems;

		CQueueItem* OutputQueue = m_Cache->GetBufferQueue(nKeyCurrent);

		while (!IsStop() && OutputQueue)
		{
			for (size_t i = 0; i < vContextInfos.size(); i++)
			{
				if (vContextInfos[i].eJob == EJob::EJ_AMixMain ||
					vContextInfos[i].eJob == EJob::EJ_AMixBranch)
				{
					if (vQueueItems.size() < vContextInfos[i].nFilterIndex + 1)
					{
						m_nPreviousPrefix = StageToPrefix(EStage::ES_Decode,
							vContextInfos[i].nFilterIndex);
						nKeyPrevious = m_nPreviousPrefix + (int)m_eStreamType;

						CQueueItem* q = m_Cache->GetBufferQueue(nKeyPrevious);
						vQueueItems.push_back(q);
					}

					while (vQueueItems[vContextInfos[i].nFilterIndex])
					{
						ret = m_Cache->Pop(
							vQueueItems[vContextInfos[i].nFilterIndex], Frame);
						if (ret < 0)
						{
							Sleep(kSleepDelay);
							continue;
						}

						m_Filter.Push(vContextInfos[i].nFilterIndex, Frame);

						if (!Frame)
							vContextInfos.erase(vContextInfos.begin() + i);

						av_frame_free(&Frame);

						break;
					}
				}
			}

			while (true)
			{
				if (!m_PopFrame)
					m_PopFrame = FFrame::AudioFrame(
						m_OutputCodecContext->frame_size,
						m_OutputCodecContext->sample_rate,
						m_OutputCodecContext->sample_fmt,
						&m_OutputCodecContext->ch_layout);

				ret = m_Filter.Pop(m_PopFrame);

				if (ret >= 0 &&
					m_PopFrame->nb_samples == m_OutputCodecContext->frame_size)
				{
					OutputQueue->Push(m_PopFrame);
					m_PopFrame = nullptr;
				}
				else
				{
					av_frame_unref(m_PopFrame);
					break;
				}
			}

			if (ret == AVERROR_EOF) break;

			ConsumeCache(nKeyCurrent);

			StageSleep();
		}
		
		// End with nullptr
		m_Cache->Push(nKeyCurrent, (AVFrame*)nullptr);

		Thread::Run();
	}

	void CFiltrate::Release()
	{
		m_Filter.Release();

		if (m_PopFrame)
			av_frame_free(&m_PopFrame);
	}

	int CFiltrate::CreateMixFilter(AVCodecContext* n_OutputCodecContext)
	{
		std::vector<AVFilterContext*> vFilterContext;
		std::vector<FContextInfo>& vContextInfos = m_Cache->GetContextInfos();

		char szBuffer[10] = { 0 };

		for (size_t i = 0; i < vContextInfos.size(); i++)
		{
			if (vContextInfos[i].eJob == EJob::EJ_AMixMain ||
				vContextInfos[i].eJob == EJob::EJ_AMixBranch)
			{
				if (!m_Filter.m_FilterGraph) m_Filter.AllocGraph(m_eStreamType);
			}

			memset(szBuffer, 0, sizeof(szBuffer));
			sprintf_s(szBuffer, sizeof(szBuffer), "buffer%zd", i);

			AVFilterContext* ctx = m_Filter.BuildContext("abuffer", szBuffer, 
				n_OutputCodecContext);
			vFilterContext.emplace_back(ctx);
		}

		if (m_Filter.m_FilterGraph)
		{
			AVFilterContext* mixCtx = m_Filter.AllocContext("amix", "mix");
			FilterMixOption(mixCtx, (int)vFilterContext.size());
			m_Filter.InitContext(mixCtx);

			for (size_t i = 0; i < vFilterContext.size(); i++)
				m_Filter.Link(vFilterContext[i], 0, mixCtx, (unsigned int)i);

			AVFilterContext* sinkCtx = m_Filter.BuildContext("abuffersink", "sink", nullptr);

			m_Filter.Link(mixCtx, 0, sinkCtx, 0);
			m_Filter.GraphConfig();
		}

		return 1;
	}

}
