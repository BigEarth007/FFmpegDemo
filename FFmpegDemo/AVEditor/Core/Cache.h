#pragma once
/*
* All AVPacket queue, AVFrame queue is managed in this class
*/


namespace aveditor
{
	struct FContextInfo
	{
		// What to do with this input context
		EJob	eJob = EJob::EJ_Normal;
		// The duration of the input context
		double	dDuration = 0.0;
		// The index of streams
		// -1: ignore the corresponding  stream in input context
		// >=0: if write this stream into output context, it's the real stream index
		int		nStreams[(int)EStreamType::EST_Max] = { -1, -1, -1 };

		// The Context name
		// If it's empty, it means it's a empty context
		std::string sName;

		// Point to CDemuxer object of this input context
		void*	Demuxer = nullptr;
		// Index of the input filter context
		unsigned int nFilterIndex = 0;

		FContextInfo() { ResetStreams(); }
		void ResetStreams()
		{
			memset((char*)nStreams, -1, sizeof(nStreams));
		}
	};

	struct AVEDITOR_API FCache
	{
		FCache();
		~FCache();

		/*
		* brief: Create a cache buffer queue for one stage
		* Parameter:
		*	const EStage& n_eStage: which stage
		* return:
		*	int: the buffer key prefix
		*/
		int CreateCache(const EStage& n_eStage, const int n_nContextIndex);
		int CreateCache(const EStage& n_eStage, const EItemType& n_eItemType, 
			const int n_nContextIndex, const EStreamType& n_eStreamType);

		// n_nKey: prefix + stream_index
		void Push(const int& n_nKey, AVFrame* n_Frame);
		// n_nKey: prefix + stream_index
		void Push(const int& n_nKey, AVPacket* n_Packet);
		// Push null packet into all streams
		void PushNullPacket(const int& n_nPrefix);
		// n_nKey: prefix + stream_index
		int Pop(const int& n_nKey, AVFrame*& n_Frame);
		// n_nKey: prefix + stream_index
		int Pop(const int& n_nKey, AVPacket*& n_Packet);

		// 
		int Pop(CQueueItem* n_QueueItem, AVFrame*& n_Frame);
		// 
		int Pop(CQueueItem* n_QueueItem, AVPacket*& n_Packet);

		// n_nKey: prefix + stream_index
		int Front(const int& n_nKey, AVFrame*& n_Frame);
		// n_nKey: prefix + stream_index
		int Front(const int& n_nKey, AVPacket*& n_Packet);
		// n_nKey: prefix + stream_index
		size_t Size(const int& n_nKey);
		// n_nKey: prefix + stream_index
		bool Empty(const int& n_nKey);

		CQueueItem* GetBufferQueue(const int& n_nKey);

		// Get stream indexes from prefix
		std::vector<unsigned int> GetStreamIndexes(const int& n_nPrefix);

		// If the stage exists or not
		bool IsStageExist(const EStage n_eStage);

		// Get the prefix of previous step;
		// with the same context index and stream index
		int GetPreviousKeyPrefix(const int& n_nKey);
		// Get the prefix of previous step;
		// with the same stream index
		std::vector<int> GetPreviousKeysPrefix(const int& n_nKey);

		void Release();

		// Add new input context
		void AddContent(FContextInfo& n_ContextInfo);
		// Set duration of the input context
		void SetContextDuration(const int& n_nContextIndex, 
			const double& n_dDuration);

		// Get information of the input context
		FContextInfo* GetContextInfo(const int& n_nContextIndex);
		std::vector<FContextInfo>& GetContextInfos();

		// Get number of input context
		int GetContextSize();

		// Get index of selected input context
		int& GetSelectedContextIndex() { return m_nSelectedContext; }
		// Set index of selected input context
		void SetSelectedContextIndex(const int n_nContextIndex);

		void DebugBufferQueue();

	protected:
		void InsertCache(const int& n_nPrefix, const EItemType& n_eItemType);
		void InsertCache(const int& n_nPrefix, const EItemType& n_eItemType,
			const EStreamType& n_eStreamType);

	protected:
		/*
		* The buffer map.
		* the key is prefix + stream_index
		*/
		std::map<int, CQueueItem*>	m_Buffer;
		// The information of input context
		std::vector<FContextInfo>	m_vContextInfo;

		// Input file context
		FFormatContext*				m_InputContext = nullptr;
		// The index of selected input context
		int							m_nSelectedContext = 0;
	};

	extern "C"
	{
		AVEDITOR_API int StageToPrefix(const EStage n_eStage, const int n_nContextIndex);
	}
}
