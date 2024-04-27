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

		// Set current duration
		void SetDuration(const double n_dDuration);

	protected:
		// Read AVPacket from queue, 
		AVPacket* ReadCache();

		void Duration2Pts();

		struct FTimeSync
		{
			double		dTimebase = -AV_NOPTS_VALUE;
			double		dTimestamp = -AV_NOPTS_VALUE;
			// 1: hold data
			// 0: empty
			int			nHold = 0;
			// stream index
			int			nStreamIndex = 0;
			// Duration to pts
			int64_t		nPts = 0;
		};

	protected:
		// The summary duration of input contexts that has been done
		double				m_dDuration = 0.0;
		// Packet cache for time synchronization
		FTimeSync			m_TimeSync[(int)EStreamType::ST_Size] = { 0 };
	};
}
