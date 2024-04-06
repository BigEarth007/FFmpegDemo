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
			EDataType& n_eItemType, void*& n_Data, const int n_nTimeout);;

		// Get size of the queue
		const size_t GetBufferSize(EStreamType n_eStreamType);

		// Set max buffer queue size
		void SetMaxBufferSize(int n_nSize);

		virtual void Release();

		// Set if buffer size limited is enabled
		void EnableBufferSizeLimited(bool n_bEnable);

	protected:
		virtual bool LimitBufferSize();

	protected:
		// queue buffer
		CAVQueue		m_qCache[(int)EStreamType::ST_Size];

		// Max buffer size
		int				m_nMaxBufferSize = 50;

		// Enable buffer size limit
		bool			m_nBufferSizeLimited = true;
	};
}
