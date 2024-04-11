#pragma once

namespace aveditor
{
	class AVEDITOR_API IAVIOHandle
	{
	public:
		/*
		* Writing data into this component
		* const EStreamType: stream type of data
		* void*: data, must be one of AVPacket/AVFrame
		* EDataType: data type
		*		DT_Packet: AVPacket
		*		DT_Frame: AVFrame
		* int: index of current batch
		*		all input file will be executed by batch
		*		and one batch may contains more than one input file
		*
		* this function is called outside for writing data
		*/
		virtual int ReceiveData(const EStreamType n_eStreamType,
			void* n_Data, EDataType n_eType, int n_nIndex = 0);

		// Pop data from queue
		int Pop(const EStreamType n_eStreamType,
			EDataType& n_eItemType, void*& n_Data, const int n_nTimeout);

		// Front data of queue
		int Front(const EStreamType n_eStreamType,
			EDataType& n_eItemType, void*& n_Data);

		// Get size of the queue
		const size_t GetBufferSize(EStreamType n_eStreamType);

		// Set max buffer queue size
		void SetMaxBufferSize(int n_nSize);

		virtual void Release();

		// Set end flag
		void SetEndFlag(const bool n_bEndFlag);
		// Get end flag, is it end now
		const bool GetEndFlag() const;

		// Set stream end flag
		void SetStreamEndFlag(const EStreamType n_eStreamType,
			const int n_nEndFlag);
		// Get stream end flag
		const int GetStreamEndFlag(const EStreamType n_eStreamType) const;

	protected:
		virtual bool LimitBufferSize();

		enum class ECompStatus
		{
			CS_Normal = 0,
			CS_Ready,
			CS_Stop,
			CS_ForceStop,
		};

		struct FBufferQueue
		{
			CAVQueue	AVQueue;
			// Is current buffer end
			int			EndFlag = 1;
		};

	protected:
		FBufferQueue	m_Cache[(int)EStreamType::ST_Size];

		// Max buffer size
		int				m_nMaxBufferSize = 80;

		// Is end of the task?
		ECompStatus		m_eStatus = ECompStatus::CS_Normal;
	};
}
