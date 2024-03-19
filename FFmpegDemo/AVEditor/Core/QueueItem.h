#pragma once
/*
* A buffer queue about AVPacket or AVFrame
*/


namespace aveditor
{
	enum class AVEDITOR_API EItemType
	{
		EIT_Packet = 0,
		EIT_Frame,
	};

#ifdef STD_QUEUE
	class AVEDITOR_API CQueueItem
	{
	public:
		CQueueItem(EItemType n_eItemType);
		~CQueueItem();

		void Push(AVFrame* n_Frame);
		void Push(AVPacket* n_Packet);
		int Pop(AVFrame*& n_Frame, const int n_nTimeout);
		int Pop(AVPacket*& n_Packet, const int n_nTimeout);
		int Front(AVFrame*& n_Frame);
		int Front(AVPacket*& n_Packet);
		size_t Size();
		bool Empty();
		void Clear();

		void Release();

	protected:
		Queue<AVFrame*>		m_qFrame;
		Queue<AVPacket*>	m_qPacket;

		EItemType 		m_eItemType = EItemType::EIT_Packet;
	};
#else
	class AVEDITOR_API CQueueItem
	{
	public:
		CQueueItem(EItemType n_eItemType);
		~CQueueItem();

		void Push(AVFrame* n_Frame);
		void Push(AVPacket* n_Packet);
		int Pop(AVFrame*& n_Frame, const int n_nTimeout);
		int Pop(AVPacket*& n_Packet, const int n_nTimeout);
		int Front(AVFrame*& n_Frame);
		int Front(AVPacket*& n_Packet);
		int Size();
		bool Empty();
		void Clear();

		void Release();

	protected:
		Queue		m_Queue;
		EItemType	m_eItemType = EItemType::EIT_Packet;
	};
#endif // STD_QUEUE

}
