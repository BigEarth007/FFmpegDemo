#include "pch.h"
#include "AVIOHandle.h"

namespace aveditor
{

	int IAVIOHandle::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		CAVQueue* pQueue = &m_qCache[(int)n_eStreamType];

		if (m_nBufferSizeLimited &&
			m_nMaxBufferSize > 0 &&
			pQueue->Size() > m_nMaxBufferSize)
		{
			int nMin = m_nMaxBufferSize / 2;
			while (LimitBufferSize() &&
				pQueue->Size() > nMin)
			{
				Sleep(kSleepDelay * 5);
			}
		}

		pQueue->Push(n_eType, n_Data);

		return 0;
	}

	int IAVIOHandle::Pop(const EStreamType n_eStreamType,
		EDataType& n_eItemType, void*& n_Data, const int n_nTimeout)
	{
		return m_qCache[(int)n_eStreamType].Pop(n_eItemType, n_Data, n_nTimeout);
	}

	const size_t IAVIOHandle::GetBufferSize(EStreamType n_eStreamType)
	{
		return m_qCache[(int)n_eStreamType].Size();
	}

	void IAVIOHandle::SetMaxBufferSize(int n_nSize)
	{
		m_nMaxBufferSize = n_nSize;
	}

	void IAVIOHandle::Release()
	{
		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_qCache[i].Clear();
		}
	}

	void IAVIOHandle::EnableBufferSizeLimited(bool n_bEnable)
	{
		m_nBufferSizeLimited = n_bEnable;
	}

	bool IAVIOHandle::LimitBufferSize()
	{
		return true;
	}

}
