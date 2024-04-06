#pragma once

namespace aveditor
{
	class AVEDITOR_API CMuxerComponent : public IComponent
	{
	public:
		CMuxerComponent();
		virtual ~CMuxerComponent();

		void Init();
		int Run();
		void Release();

		/*
		* Writing data into this component
		* const EStreamType: stream type of data
		* void*: data, must be one of AVPacket/AVFrame
		* int: data type
		*		0: AVPacket
		*		1: AVFrame
		* int: index of current batch
		*		all input file will be executed by batch
		*		and one batch may contains more than one input file
		*
		* this function is called outside for writing data
		*/
		virtual int ReceiveData(const EStreamType n_eStreamType,
			void* n_Data, EDataType n_eType, int n_nIndex = 0);

		// Add duration of input context that has been done
		void AddDuration(const double n_dDuration);

	protected:
		// Read AVPacket from queue, 
		bool ReadCache();

		struct FTimeSync
		{
			AVPacket*	Packet = nullptr;
			double		dTimebase = 0.0f;
			double		dTimestamp = 0.0f;
			// -1: not use
			// 0: using
			// 1: finished
			int			nStatus = -1;
			// stream index
			int			nStream = 0;

			// Is there a packet data in storage
			bool Writable() const { return nStatus == 0 && !Packet; }
			// Is there a packet that can be read
			bool Readable() const { return nStatus == 0 && Packet; }
			void Fill(AVPacket* n_Packet)
			{
				dTimestamp = n_Packet->pts * dTimebase;
				Packet = n_Packet;
			}
		};

	protected:
		// The summary duration of input contexts that has been done
		double				m_dDuration = 0.0;
		// Packet cache for time synchronization
		FTimeSync			m_TimeSync[(int)EStreamType::ST_Size] = { 0 };

		// Index of stream with min timestamp
		int					m_nMinIndex = 0;

		int					m_nReadSize = 0;
		int					m_nStreamSize = 0;
	};
}
