#pragma once


namespace aveditor
{
	struct AVEDITOR_API FFilter
	{
		FFilter() = default;
		~FFilter();

		/* Create a new filter graph, which will contain all the filters. */
		AVFilterGraph* AllocGraph(EStreamType n_eStreamType);

		/* Create the filter. */
		AVFilterContext* AllocContext(const char* n_szFilterName,
			const char* n_szContextName);

		/* Create the filter. */
		AVFilterContext* BuildContext(const char* n_szFilterName,
			const char* n_szContextName, AVCodecContext* n_CodecContext);

		/* Now initialize the filter. */
		void InitContext(AVFilterContext* n_FilterContext, 
			AVDictionary* n_Options);

		/* Now initialize the filter. */
		/* n_szArgs: a string of the form key1=value1:key2=value2....*/
		void InitContext(AVFilterContext* n_FilterContext,
			const char* n_szArgs);

		/* Now initialize the filter. */
		void InitContext(AVFilterContext* n_FilterContext);

		/* Connect the filters; */
		void Link(AVFilterContext* n_SrcFilterContext, unsigned n_nSrcpad,
			AVFilterContext* n_DstFilterContext, unsigned n_nDstpad);

		/* Configure the graph. */
		void GraphConfig(void* arg_ctx = nullptr);

		/* Find filter context */
		AVFilterContext* FindFilterContext(const char* n_szInstName);

		/* Send the frame to the input of the filter graph. */
		void Push(AVFilterContext* n_FilterContext, AVFrame* n_Frame);
		//void Push(const char* n_FilterName, AVFrame* n_Frame);
		void Push(const unsigned int n_nIndex, AVFrame* n_Frame);

		/* Get all the filtered output that is available. */
		int Pop(AVFrame* n_Frame);

		// Get stream type
		EStreamType GetStreamType() { return m_eStreamType; }

		void Release();

		AVFilterGraph* m_FilterGraph = nullptr;


	protected:
		// Stream type
		EStreamType		m_eStreamType = EStreamType::EST_Max;

		// Output filter context
		AVFilterContext* m_SinkFilter = nullptr;

		// The filter context for adding frame
		std::vector<AVFilterContext*>	m_vInputFilters;
	};

	extern "C"
	{
		AVEDITOR_API void FilterVideoOption(AVFilterContext* n_FilterContext,
			AVCodecContext* n_CodecContext);
		AVEDITOR_API void FilterAudioOption(AVFilterContext* n_FilterContext,
			AVCodecContext* n_CodecContext);

		AVEDITOR_API void FilterMixOption(AVFilterContext* n_FilterContext,
			const int n_nInput);
	}
}
