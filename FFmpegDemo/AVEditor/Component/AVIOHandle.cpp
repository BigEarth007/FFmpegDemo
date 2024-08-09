#include "pch.h"
#include "AVIOHandle.h"

namespace aveditor
{

	int IAVIOHandle::ReceiveData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		CAVQueue* pQueue = &m_Cache[(int)n_eStreamType].AVQueue;

		if (m_nMaxBufferSize > 0 &&
			GetBufferSize(n_eStreamType) > m_nMaxBufferSize)
		{
			EStreamType eStreamMinSize = EStreamType::ST_Size;
			size_t nSize = m_nMaxBufferSize;
			for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
			{
				// get min buffer size of other stream buffer
				EStreamType st = (EStreamType)i;
				if (GetStreamEndFlag(st) == 0 &&
					st != n_eStreamType &&
					GetBufferSize(st) < nSize)
				{
					eStreamMinSize = st;
					nSize = GetBufferSize(st);
				}
			}

			if (eStreamMinSize != EStreamType::ST_Size && nSize > 10)
			{
				int nMin = m_nMaxBufferSize / 2;
				while (LimitBufferSize() &&
					GetBufferSize(eStreamMinSize) > 10 &&
					pQueue->Size() > nMin)
				{
					Sleep(kSleepDelay * 5);
				}
			}
		}

		pQueue->Push(n_eType, n_Data);

		return 0;
	}

	int IAVIOHandle::Pop(const EStreamType n_eStreamType,
		EDataType& n_eItemType, void*& n_Data, const int n_nTimeout)
	{
		return m_Cache[(int)n_eStreamType].AVQueue.Pop(n_eItemType, n_Data, n_nTimeout);
	}

	int IAVIOHandle::Front(const EStreamType n_eStreamType, 
		EDataType& n_eItemType, void*& n_Data)
	{
		return m_Cache[(int)n_eStreamType].AVQueue.Front(n_eItemType, n_Data);
	}

	const size_t IAVIOHandle::GetBufferSize(EStreamType n_eStreamType)
	{
		return m_Cache[(int)n_eStreamType].AVQueue.Size();
	}

	void IAVIOHandle::SetMaxBufferSize(int n_nSize)
	{
		m_nMaxBufferSize = n_nSize;
	}

	void IAVIOHandle::Release()
	{
		for (size_t i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_Cache[i].AVQueue.Clear();
		}
	}

	void IAVIOHandle::SetEndFlag(const bool n_bEndFlag)
	{
		if (n_bEndFlag)
			m_eStatus = ECompStatus::CS_Stop;
		else if (m_eStatus != ECompStatus::CS_ForceStop)
			m_eStatus = ECompStatus::CS_Ready;
	}

	const bool IAVIOHandle::GetEndFlag() const
	{
		return m_eStatus == ECompStatus::CS_Stop;
	}

	void IAVIOHandle::ResetStreamEndFlag()
	{
		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			m_Cache[i].EndFlag = 1;
		}
	}

	void IAVIOHandle::SetStreamEndFlag(const EStreamType n_eStreamType,
		const int n_nEndFlag)
	{
		m_Cache[(int)n_eStreamType].EndFlag = n_nEndFlag;

		bool bFlag = true;

		for (int i = 0; i < (int)EStreamType::ST_Size; i++)
		{
			bFlag &= m_Cache[i].EndFlag != 0;
		}

		SetEndFlag(bFlag);
	}

	const int IAVIOHandle::GetStreamEndFlag(const EStreamType n_eStreamType) const
	{
		return m_Cache[(int)n_eStreamType].EndFlag;
	}

	bool IAVIOHandle::LimitBufferSize()
	{
		return true;
	}

}
