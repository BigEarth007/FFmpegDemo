#include "pch.h"
#include "QueueItem.h"


namespace aveditor
{
#ifdef STD_QUEUE
	CQueueItem::CQueueItem(EItemType n_eItemType)
	{
		m_eItemType = n_eItemType;
	}

	CQueueItem::~CQueueItem()
	{
		Release();
	}

	void CQueueItem::Push(AVFrame* n_Frame)
	{
		if (m_eItemType == EItemType::EIT_Frame)
			m_qFrame.Push(n_Frame);
	}

	void CQueueItem::Push(AVPacket* n_Packet)
	{
		if (m_eItemType == EItemType::EIT_Packet)
			m_qPacket.Push(n_Packet);
	}

	int CQueueItem::Pop(AVFrame*& n_Frame, const int n_nTimeout)
	{
		if (m_eItemType == EItemType::EIT_Frame)
			return m_qFrame.Pop(n_Frame, n_nTimeout);
		
		return -1;
	}

	int CQueueItem::Pop(AVPacket*& n_Packet, const int n_nTimeout)
	{
		if (m_eItemType == EItemType::EIT_Packet)
			return m_qPacket.Pop(n_Packet, n_nTimeout);
		
		return -1;
	}

	int CQueueItem::Front(AVFrame*& n_Frame)
	{
		if (m_eItemType == EItemType::EIT_Frame)
			return m_qFrame.Front(n_Frame);
		
		return -1;
	}

	int CQueueItem::Front(AVPacket*& n_Packet)
	{
		if (m_eItemType == EItemType::EIT_Packet)
			return m_qPacket.Front(n_Packet);

		return -1;
	}

	size_t CQueueItem::Size()
	{
		if (m_eItemType == EItemType::EIT_Frame)
			return m_qFrame.Size();
		if (m_eItemType == EItemType::EIT_Packet)
			return m_qPacket.Size();
		
		return 0;
	}

	bool CQueueItem::Empty()
	{
		if (m_eItemType == EItemType::EIT_Frame)
			return m_qFrame.Empty();
		if (m_eItemType == EItemType::EIT_Packet)
			return m_qPacket.Empty();

		return true;
	}

	void CQueueItem::Clear()
	{
		if (m_eItemType == EItemType::EIT_Frame)
		{
			m_qFrame.Clear(
				[this](AVFrame* n_Frame) {
					av_frame_free(&n_Frame);
				}
			);
		}

		if (m_eItemType == EItemType::EIT_Packet)
		{
			m_qPacket.Clear(
				[this](AVPacket* n_Packet) {
					av_packet_free(&n_Packet);
				}
			);
		}
	}

	void CQueueItem::Release()
	{
		Clear();
	}
#else
	CQueueItem::CQueueItem(EItemType n_eItemType)
	{
		m_eItemType = n_eItemType;
	}

	CQueueItem::~CQueueItem()
	{
		Release();
	}

	void CQueueItem::Push(AVFrame* n_Frame)
	{
		m_Queue.Push(n_Frame);
	}

	void CQueueItem::Push(AVPacket* n_Packet)
	{
		m_Queue.Push(n_Packet);
	}

	int CQueueItem::Pop(AVFrame*& n_Frame, const int n_nTimeout)
	{
		return m_Queue.Pop((void*&)n_Frame, n_nTimeout);
	}

	int CQueueItem::Pop(AVPacket*& n_Packet, const int n_nTimeout)
	{
		return m_Queue.Pop((void*&)n_Packet, n_nTimeout);
	}

	int CQueueItem::Front(AVFrame*& n_Frame)
	{
		return m_Queue.Front((void*&)n_Frame);
	}

	int CQueueItem::Front(AVPacket*& n_Packet)
	{
		return m_Queue.Front((void*&)n_Packet);
	}

	int CQueueItem::Size()
	{
		return m_Queue.Size();
	}

	bool CQueueItem::Empty()
	{
		return m_Queue.Empty();
	}

	void CQueueItem::Clear()
	{
		switch (m_eItemType)
		{
		case aveditor::EItemType::EIT_Packet:
		{
			m_Queue.Clear(
				[this](void* n_Item) {
					AVPacket* Packet = (AVPacket*)n_Item;
					av_packet_free(&Packet);
				}
			);
		}
		break;
		case aveditor::EItemType::EIT_Frame:
		{
			m_Queue.Clear(
				[this](void* n_Item) {
					AVFrame* Frame = (AVFrame*)n_Item;
					av_frame_free(&Frame);
				}
			);
		}
		break;
		}
	}

	void CQueueItem::Release()
	{
		Clear();
	}
#endif // STD_QUEUE

}
