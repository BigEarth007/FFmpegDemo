#include "pch.h"
#include "QueueItem.h"


namespace aveditor
{
#ifdef STD_QUEUE
	CQueueItem::CQueueItem(EItemType n_eItemType)
	{
		m_eItemType = n_eItemType;

		switch (m_eItemType)
		{
		case aveditor::EItemType::EIT_Packet:
			m_qBuffer = (Queue<void*>*)(new Queue<AVPacket*>());
			break;
		case aveditor::EItemType::EIT_Frame:
			m_qBuffer = (Queue<void*>*)(new Queue<AVFrame*>());
			break;
		default:
			break;
		}
	}

	CQueueItem::~CQueueItem()
	{
		Release();
	}

	void CQueueItem::Push(void* n_AVData)
	{
		if (m_qBuffer) m_qBuffer->Push(n_AVData);
	}

	int CQueueItem::Pop(void*& n_AVData, const int n_nTimeout)
	{
		if (!m_qBuffer) return -1;
		return m_qBuffer->Pop(n_AVData, n_nTimeout);		
	}

	int CQueueItem::Pop(AVFrame*& n_Frame, const int n_nTimeout)
	{
		if (!m_qBuffer) return -1;

		void* pData = nullptr;
		int ret = m_qBuffer->Pop(pData, n_nTimeout);
		n_Frame = (AVFrame*)pData;

		return ret;
	}

	int CQueueItem::Pop(AVPacket*& n_Packet, const int n_nTimeout)
	{
		if (!m_qBuffer) return -1;

		void* pData = nullptr;
		int ret = m_qBuffer->Pop(pData, n_nTimeout);
		n_Packet = (AVPacket*)pData;

		return ret;
	}

	int CQueueItem::Front(void*& n_AVData)
	{
		if (!m_qBuffer) return -1;
		return m_qBuffer->Front(n_AVData);
	}

	int CQueueItem::Front(AVFrame*& n_Frame)
	{
		if (!m_qBuffer) return -1;

		void* pData = nullptr;
		int ret = m_qBuffer->Front(pData);
		n_Frame = (AVFrame*)pData;

		return ret;
	}

	int CQueueItem::Front(AVPacket*& n_Packet)
	{
		if (!m_qBuffer) return -1;

		void* pData = nullptr;
		int ret = m_qBuffer->Front(pData);
		n_Packet = (AVPacket*)pData;

		return ret;
	}

	size_t CQueueItem::Size()
	{
		if (m_qBuffer) return m_qBuffer->Size();
		
		return 0;
	}

	bool CQueueItem::Empty()
	{
		if (m_qBuffer) return m_qBuffer->Empty();

		return true;
	}

	void CQueueItem::Clear()
	{
		m_qBuffer->Clear(
			[this](void* n_AVData) {
				if (m_eItemType == EItemType::EIT_Frame)
				{
					AVFrame* Frame = (AVFrame*)n_AVData;
					av_frame_free(&Frame);
				}
				else if (m_eItemType == EItemType::EIT_Packet)
				{
					AVPacket* Packet = (AVPacket*)n_AVData;
					av_packet_free(&Packet);
				}
			}
		);
	}

	void CQueueItem::Release()
	{
		Clear();

		if (m_qBuffer)
		{
			delete m_qBuffer;
			m_qBuffer = nullptr;
		}
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
