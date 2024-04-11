#include "pch.h"
#include "AudioMixComponent.h"

namespace aveditor
{
	CAudioMixComponent::CAudioMixComponent()
	{
		SetCompID(ECompID::EI_Filter);
	}

	CAudioMixComponent::~CAudioMixComponent()
	{
		Release();
	}

	void CAudioMixComponent::Init(int n_nBatchIndex)
	{
		IComponent::Init(n_nBatchIndex);

		FFormatContext& OutputCodecContext = m_Editor->GetOutputContext()->GetContext();
		m_OutputCodecContext = OutputCodecContext.GetCodecContext(EStreamType::ST_Audio);

		auto vInputs = m_Editor->GetInputContextsByBatch(n_nBatchIndex);
		CreateFilter(vInputs);
	}

	int CAudioMixComponent::Run()
	{
		int ret = 0;

		if (GetEndFlag() || !IsValid()) return AVERROR_EOF;

		if (!m_PopFrame)
			m_PopFrame = FFrame::AudioFrame(
				m_OutputCodecContext->frame_size,
				m_OutputCodecContext->sample_rate,
				m_OutputCodecContext->sample_fmt,
				&m_OutputCodecContext->ch_layout);

		ret = m_Filter.Pop(1, m_PopFrame);

		if (ret >= 0 &&
			m_PopFrame->nb_samples == m_OutputCodecContext->frame_size)
		{
			ret = WriteData(EStreamType::ST_Audio,
					m_PopFrame, EDataType::DT_Frame, 0);
			m_PopFrame = nullptr;
		}
		else
		{
			av_frame_unref(m_PopFrame);
		}

		if (ret == AVERROR_EOF)
		{
			WriteData(EStreamType::ST_Audio, nullptr, EDataType::DT_Frame, 0);
			SetStreamEndFlag(EStreamType::ST_Audio, 1);
		}

		return ret;
	}

	void CAudioMixComponent::Release()
	{
		m_Filter.Release();
		if (m_PopFrame) av_frame_free(&m_PopFrame);

		SetStreamEndFlag(EStreamType::ST_Audio, 1);

		IComponent::Release();
	}

	bool CAudioMixComponent::IsValid() const
	{
		return m_Filter.m_FilterGraph;
	}

	int CAudioMixComponent::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (IsValid() && 
			n_eType == EDataType::DT_Frame && 
			n_eStreamType == EStreamType::ST_Audio)
		{
			AVFrame* Frame = (AVFrame*)n_Data;

			m_Filter.Push(n_nIndex + 2, Frame);
			av_frame_free(&Frame);
		}
		else
		{
			if (!n_Data) SetStreamEndFlag(n_eStreamType, 1);
			ret = WriteData(n_eStreamType, n_Data, n_eType, n_nIndex);
		}

		return ret;
	}

	int CAudioMixComponent::CreateFilter(const std::vector<int>& n_vContextIndex)
	{
		if (!IsValid())
		{
			std::vector<int> vContextIndex = n_vContextIndex;

			// Remove index of input context that not in mix task
			for (int i = 0; i < (int)vContextIndex.size();)
			{
				auto Input = m_Editor->GetInputContext(i);
				if (Input->GetTask() != ETask::T_AMixMain &&
					Input->GetTask() != ETask::T_AMixBranch)
				{
					vContextIndex.erase(vContextIndex.begin() + i);
				}
				else
				{
					i++;
				}
			}

			if (vContextIndex.size() >= 2)
			{
				m_Filter.AllocGraph(EStreamType::ST_Audio);

				AVFilterContext* mixCtx = m_Filter.AllocContext("amix", "mix");
				FilterMixOption(mixCtx, (int)vContextIndex.size());
				m_Filter.InitContext(mixCtx);

				AVFilterContext* sinkCtx = m_Filter.BuildContext("abuffersink", 
					"sink", nullptr);
				m_Filter.Link(mixCtx, 0, sinkCtx, 0);

				char szBuffer[10] = { 0 };

				for (size_t i = 0; i < vContextIndex.size(); i++)
				{
					CInputContext* Input = m_Editor->GetInputContext(vContextIndex[i]);
					int nSubnumber = Input->GetSubnumber();

					memset(szBuffer, 0, sizeof(szBuffer));
					sprintf_s(szBuffer, sizeof(szBuffer), "buffer%d", nSubnumber);

					AVFilterContext* ctx = m_Filter.BuildContext("abuffer", szBuffer,
						m_OutputCodecContext);
					m_Filter.Link(ctx, 0, mixCtx, nSubnumber);
				}

				m_Filter.GraphConfig();
			}
		}

		if (IsValid()) SetStreamEndFlag(EStreamType::ST_Audio, 0);

		return 0;
	}

}