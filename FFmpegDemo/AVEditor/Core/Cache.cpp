#include "pch.h"
#include "Cache.h"


namespace aveditor
{
	FCache::FCache()
	{
	}

	FCache::~FCache()
	{
		Release();
	}

	int FCache::CreateCache(const EStage& n_eStage, const int n_nContextIndex)
	{
		int nPrefix = StageToPrefix(n_eStage, n_nContextIndex);

		switch (n_eStage)
		{
		case EStage::ES_Demux:
		case EStage::ES_Encode:
			InsertCache(nPrefix, EItemType::EIT_Packet);
			break;
		case EStage::ES_Filter:
		case EStage::ES_Decode:
			InsertCache(nPrefix, EItemType::EIT_Frame);
			break;
		default:
			break;
		}

		return nPrefix;
	}

	int FCache::CreateCache(const EStage& n_eStage, const EItemType& n_eItemType, 
		const int n_nContextIndex, const EStreamType& n_eStreamType)
	{
		int nPrefix = StageToPrefix(n_eStage, n_nContextIndex);
		InsertCache(nPrefix, n_eItemType, n_eStreamType);

		return nPrefix;
	}

	void FCache::Push(const int& n_nKey, AVFrame* n_Frame)
	{
		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			itr->second->Push(n_Frame);
		}
	}

	void FCache::Push(const int& n_nKey, AVPacket* n_Packet)
	{
		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			itr->second->Push(n_Packet);
		}
	}

	void FCache::PushNullPacket(const int& n_nPrefix)
	{
		int nKey = 0;
		int nContextIndex = n_nPrefix % kEditorFactor / kEditorIndexFactor;
		ThrowExceptionExpr(nContextIndex >= m_vContextInfo.size(),
			"Invalid parameter.\n");

		for (int i = 0;i < (int)EStreamType::EST_Max; i++)
		{
			if (m_vContextInfo[nContextIndex].nStreams[i] != -1)
			{
				nKey = n_nPrefix + i;
				Push(nKey, (AVPacket*)nullptr);
			}
		}
	}

	int FCache::Pop(const int& n_nKey, AVFrame*& n_Frame)
	{
		int nResult = -1;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			nResult = itr->second->Pop(n_Frame, kSleepTimeout);
		}

		return nResult;
	}

	int FCache::Pop(const int& n_nKey, AVPacket*& n_Packet)
	{
		int nResult = -1;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			nResult = itr->second->Pop(n_Packet, kSleepTimeout);
		}

		return nResult;
	}

	int FCache::Pop(CQueueItem* n_QueueItem, AVFrame*& n_Frame)
	{
		if (!n_QueueItem) return -1;
		return n_QueueItem->Pop(n_Frame, kSleepTimeout);
	}

	int FCache::Pop(CQueueItem* n_QueueItem, AVPacket*& n_Packet)
	{
		if (!n_QueueItem) return -1;
		return n_QueueItem->Pop(n_Packet, kSleepTimeout);
	}

	int FCache::Front(const int& n_nKey, AVFrame*& n_Frame)
	{
		int nResult = -1;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			nResult = itr->second->Front(n_Frame);
		}

		return nResult;
	}

	int FCache::Front(const int& n_nKey, AVPacket*& n_Packet)
	{
		int nResult = -1;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			nResult = itr->second->Front(n_Packet);
		}

		return nResult;
	}

	size_t FCache::Size(const int& n_nKey)
	{
		size_t nResult = 0;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			nResult = itr->second->Size();
		}

		return nResult;
	}

	bool FCache::Empty(const int& n_nKey)
	{
		bool bResult = true;

		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
		{
			bResult = itr->second->Empty();
		}

		return bResult;
	}

	CQueueItem* FCache::GetBufferQueue(const int& n_nKey)
	{
		auto itr = m_Buffer.find(n_nKey);
		if (itr != m_Buffer.end())
			return itr->second;

		return nullptr;
	}

	std::vector<unsigned int> FCache::GetStreamIndexes(const int& n_nPrefix)
	{
		std::vector<unsigned int> vResult;

		for (auto itr = m_Buffer.begin();itr != m_Buffer.end(); itr++)
		{
			if (itr->first / kEditorFactor == n_nPrefix)
				vResult.emplace_back(itr->first % kEditorFactor);
		}

		return vResult;
	}

	bool FCache::IsStageExist(const EStage n_eStage)
	{
		bool bResult = false;

		for (auto itr = m_Buffer.begin(); itr != m_Buffer.end(); itr++)
		{
			if (itr->first / kEditorFactor == (int)n_eStage)
			{
				bResult = true;
				break;
			}
		}

		return bResult;
	}

	int FCache::GetPreviousKeyPrefix(const int& n_nKey)
	{
		int nResult = -1;
		int nPrefix = n_nKey / kEditorFactor - 1;
		int nSuffix = n_nKey % kEditorFactor;

		if (nSuffix % kEditorIndexFactor == (int)EStreamType::EST_Max) 
			return nResult;

		while (nPrefix > 0)
		{
			auto itr = m_Buffer.find(nPrefix * kEditorFactor + nSuffix);
			if (itr != m_Buffer.end())
			{
				nResult = nPrefix * kEditorFactor +
					nSuffix / kEditorIndexFactor * kEditorIndexFactor;
				break;
			}

			nPrefix--;
		}

		return nResult;
	}

	std::vector<int> FCache::GetPreviousKeysPrefix(const int& n_nKey)
	{
		std::vector<int> vResult;
		int nPrefix = n_nKey / kEditorFactor - 1;
		int nSuffix = 0;
		int nStream = n_nKey % kEditorIndexFactor;

		while (nPrefix > 0)
		{
			int nKey = nPrefix * kEditorFactor + nSuffix * kEditorIndexFactor + nStream;
			auto itr = m_Buffer.find(nKey);

			if (itr == m_Buffer.end())
			{
				if (nSuffix > 0) break;

				nPrefix--;
				continue;
			}

			vResult.emplace_back(nKey);
			nSuffix++;
		}

		return vResult;
	}

	void FCache::Release()
	{
		ReleaseMap(m_Buffer);
		m_vContextInfo.clear();
		m_nSelectedContext = 0;
	}

	void FCache::AddContent(FContextInfo& n_ContextInfo)
	{
		m_vContextInfo.emplace_back(n_ContextInfo);
	}

	void FCache::SetContextDuration(const int& n_nContextIndex,
		const double& n_dDuration)
	{
		if (n_nContextIndex < m_vContextInfo.size())
			m_vContextInfo[n_nContextIndex].dDuration = n_dDuration;
	}

	FContextInfo* FCache::GetContextInfo(const int& n_nContextIndex)
	{
		if (n_nContextIndex < m_vContextInfo.size())
			return &m_vContextInfo[n_nContextIndex];

		return nullptr;
	}

	std::vector<aveditor::FContextInfo>& FCache::GetContextInfos()
	{
		return m_vContextInfo;
	}

	int FCache::GetContextSize()
	{
		return (int)m_vContextInfo.size();
	}

	void FCache::SetSelectedContextIndex(const int n_nContextIndex)
	{
		if (n_nContextIndex < m_vContextInfo.size())
			m_nSelectedContext = n_nContextIndex;
	}

	void FCache::DebugBufferQueue()
	{
#if _DEBUG
		for (auto itr = m_Buffer.begin(); itr != m_Buffer.end(); itr++)
		{
			LogInfo("Queue: %d ----> size: %zd.\n", itr->first, itr->second->Size());
		}

		LogInfo("====================================================\n\n");
#endif
	}

	void FCache::InsertCache(const int& n_nPrefix, const EItemType& n_eItemType)
	{
		int nContextIndex = n_nPrefix % kEditorFactor / kEditorIndexFactor;
		ThrowExceptionExpr(nContextIndex >= m_vContextInfo.size(),
			"Invalid parameter.\n");

		for (int i = 0; i < (int)EStreamType::EST_Max; i++)
		{
			if (m_vContextInfo[nContextIndex].nStreams[i] != -1)
				m_Buffer.insert(
					{
					n_nPrefix + i,
					new CQueueItem(n_eItemType)
					});
		}
	}

	void FCache::InsertCache(const int& n_nPrefix, const EItemType& n_eItemType, 
		const EStreamType& n_eStreamType)
	{
		m_Buffer.insert({ n_nPrefix + (int)n_eStreamType, new CQueueItem(n_eItemType)});
	}

	AVEDITOR_API int StageToPrefix(const EStage n_eStage, const int n_nContextIndex)
	{
		return (int)n_eStage * kEditorFactor + n_nContextIndex * kEditorIndexFactor;
	}

}