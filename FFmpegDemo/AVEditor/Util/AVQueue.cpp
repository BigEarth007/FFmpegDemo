#include "pch.h"
#include "AVQueue.h"


namespace aveditor
{
	CAVQueue::CAVQueue()
	{
	}

	CAVQueue::~CAVQueue()
	{
		Release();
	}

	void CAVQueue::SetDataType(const EDataType& n_eDataType)
	{
		m_eItemType = n_eDataType;
	}

	const EDataType CAVQueue::GetDataType() const
	{
		return m_eItemType;
	}

	void CAVQueue::Push(void* n_Data)
	{
		m_qBuffer.Push(n_Data);
	}

	void CAVQueue::Push(const EDataType& n_eDataType, void* n_Data)
	{
		if (m_eItemType == EDataType::DT_None)
		 	m_eItemType = n_eDataType;
		 
		m_qBuffer.Push(n_Data);
	}

	int CAVQueue::Pop(EDataType& n_eItemType,
		void*& n_Data, const int n_nTimeout)
	{
		if (m_eItemType == EDataType::DT_None) return -1;
		n_eItemType = m_eItemType;
		return m_qBuffer.Pop(n_Data, n_nTimeout);
	}

	int CAVQueue::Pop(void*& n_Data, const int n_nTimeout)
	{
		if (m_eItemType == EDataType::DT_None) return -1;
		return m_qBuffer.Pop(n_Data, n_nTimeout);
	}

	int CAVQueue::Front(void*& n_Data)
	{
		if (m_eItemType == EDataType::DT_None) return -1;
		return m_qBuffer.Front(n_Data);
	}

	int CAVQueue::Front(EDataType& n_eItemType, void*& n_Data)
	{
		if (m_eItemType == EDataType::DT_None) return -1;
		n_eItemType = m_eItemType;
		return m_qBuffer.Front(n_Data);
	}

	size_t CAVQueue::Size()
	{
		return m_qBuffer.Size();
	}

	bool CAVQueue::Empty()
	{
		return m_qBuffer.Empty();
	}

	void CAVQueue::Clear()
	{
		m_qBuffer.Clear(
			[this](void* n_Data) {
				AVFreeData(m_eItemType, n_Data);
			}
		);

		m_eItemType = EDataType::DT_None;
	}

	void CAVQueue::Release()
	{
		Clear();
	}
}
