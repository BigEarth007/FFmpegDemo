#include "pch.h"
#include "Filter.h"


namespace aveditor
{
	FFilter::~FFilter()
	{
		Release();
	}

	AVFilterGraph* FFilter::AllocGraph(EStreamType n_eStreamType)
	{
		if (!m_FilterGraph) m_FilterGraph = avfilter_graph_alloc();
		ThrowExceptionExpr(!m_FilterGraph, "Fail to create filter graph.\n");

		m_eStreamType = n_eStreamType;

		return m_FilterGraph;
	}

	AVFilterContext* FFilter::AllocContext(const char* n_szFilterName,
		const char* n_szContextName)
	{
		ThrowExceptionExpr(!m_FilterGraph,
			"You should call function AllocGraph first.\n");

		const AVFilter* Filter = avfilter_get_by_name(n_szFilterName);
		ThrowExceptionExpr(!Filter, "Fail to find the filter: %s.\n", n_szFilterName);

		AVFilterContext* Context = avfilter_graph_alloc_filter(m_FilterGraph, 
			Filter, n_szContextName);

		ThrowExceptionExpr(!Context, 
			"Fail to allocate the AVFilterContext instance: .\n", n_szContextName);

		return Context;
	}

	AVFilterContext* FFilter::BuildContext(const char* n_szFilterName, 
		const char* n_szContextName, AVCodecContext* n_CodecContext)
	{
		AVFilterContext* FilterContext = AllocContext(n_szFilterName, n_szContextName);

		if (n_CodecContext)
		{
			if (m_eStreamType == EStreamType::EST_Video)
				FilterVideoOption(FilterContext, n_CodecContext);
			else if (m_eStreamType == EStreamType::EST_Audio)
				FilterAudioOption(FilterContext, n_CodecContext);
		}

		InitContext(FilterContext);

		return FilterContext;
	}

	void FFilter::InitContext(AVFilterContext* n_FilterContext,
		AVDictionary* n_Options)
	{
		int ret = avfilter_init_dict(n_FilterContext, &n_Options);
		if (n_Options) av_dict_free(&n_Options);

		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to initialize the filter.\n");
	}

	void FFilter::InitContext(AVFilterContext* n_FilterContext, 
		const char* n_szArgs)
	{
		int ret = avfilter_init_str(n_FilterContext, n_szArgs);

		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to initialize the filter.\n");
	}

	void FFilter::InitContext(AVFilterContext* n_FilterContext)
	{
		InitContext(n_FilterContext, (const char*)nullptr);
	}

	void FFilter::Link(AVFilterContext* n_SrcFilterContext, unsigned n_nSrcpad, 
		AVFilterContext* n_DstFilterContext, unsigned n_nDstpad)
	{
		int ret = avfilter_link(n_SrcFilterContext, n_nSrcpad,
			n_DstFilterContext, n_nDstpad);

		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to connecting filters.\n");
	}

	void FFilter::GraphConfig(void* arg_ctx /*= nullptr*/)
	{
		int ret = avfilter_graph_config(m_FilterGraph, arg_ctx);

		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to config the filter graph.\n");

		for (unsigned int i = 0; i < m_FilterGraph->nb_filters; i++)
		{
			if (strstr(m_FilterGraph->filters[i]->filter->name, "buffersink"))
				m_SinkFilter = m_FilterGraph->filters[i];
			else if (strstr(m_FilterGraph->filters[i]->filter->name, "buffer"))
				m_vInputFilters.emplace_back(m_FilterGraph->filters[i]);
		}
	}

	AVFilterContext* FFilter::FindFilterContext(const char* n_szInstName)
	{
		return avfilter_graph_get_filter(m_FilterGraph, n_szInstName);
	}

	void FFilter::Push(AVFilterContext* n_FilterContext, AVFrame* n_Frame)
	{
		int ret = av_buffersrc_add_frame(n_FilterContext, n_Frame);

		ThrowExceptionCodeExpr(ret < 0, ret, 
			"Fail to submit the frame to the filtergraph.\n");
	}

	//void FFilter::Push(const char* n_FilterName, AVFrame* n_Frame)
	//{
	//	if (!m_FilterGraph) return;

	//	for (unsigned int i = 0;i < m_FilterGraph->nb_filters; i++)
	//	{
	//		if (0 == strcmp(n_FilterName, m_FilterGraph->filters[i]->name))
	//		{
	//			Push(m_FilterGraph->filters[i], n_Frame);
	//			break;
	//		}
	//	}
	//}

	void FFilter::Push(const unsigned int n_nIndex, AVFrame* n_Frame)
	{
		if (n_nIndex < m_vInputFilters.size())
			Push(m_vInputFilters[n_nIndex], n_Frame);
	}

	int FFilter::Pop(AVFrame* n_Frame)
	{
		if (!m_FilterGraph) return -1;

		int ret = av_buffersink_get_frame(m_SinkFilter, n_Frame);

		return ret;
	}

	void FFilter::Release()
	{
		if (m_FilterGraph)
		{
			avfilter_graph_free(&m_FilterGraph);
			m_FilterGraph = nullptr;
		}
	}

	AVEDITOR_API void FilterVideoOption(AVFilterContext* n_FilterContext, 
		AVCodecContext* n_CodecContext)
	{
		av_opt_set_int(n_FilterContext, "width", n_CodecContext->width,
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set_int(n_FilterContext, "height", n_CodecContext->height,
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set_q(n_FilterContext, "time_base", n_CodecContext->time_base,
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set(n_FilterContext, "pix_fmt", 
			av_get_pix_fmt_name(n_CodecContext->pix_fmt), AV_OPT_SEARCH_CHILDREN);
	}

	AVEDITOR_API void FilterAudioOption(AVFilterContext* n_FilterContext, 
		AVCodecContext* n_CodecContext)
	{
		char szLayout[64] = { 0 };
		av_channel_layout_describe(&n_CodecContext->ch_layout, szLayout, 
			sizeof(szLayout));

		av_opt_set(n_FilterContext, "channel_layout", szLayout,
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set(n_FilterContext, "sample_fmt", 
			av_get_sample_fmt_name(n_CodecContext->sample_fmt),
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set_q(n_FilterContext, "time_base", n_CodecContext->time_base,
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set_int(n_FilterContext, "sample_rate", n_CodecContext->sample_rate, 
			AV_OPT_SEARCH_CHILDREN);
	}

	AVEDITOR_API void FilterMixOption(AVFilterContext* n_FilterContext, 
		const int n_nInput)
	{
		av_opt_set_int(n_FilterContext, "inputs", n_nInput, AV_OPT_SEARCH_CHILDREN);
		av_opt_set_int(n_FilterContext, "dropout_transition", 0, 
			AV_OPT_SEARCH_CHILDREN);
		av_opt_set(n_FilterContext, "duration", "longest",
			AV_OPT_SEARCH_CHILDREN);
	}

}
