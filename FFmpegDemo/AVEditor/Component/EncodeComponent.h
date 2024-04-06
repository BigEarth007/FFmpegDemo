#pragma once

namespace aveditor
{
	class AVEDITOR_API CEncodeComponent : public IComponent
	{
	public:
		CEncodeComponent();
		~CEncodeComponent();

		void Init();
		int Run(EStreamType n_eStreamType);
		void Release();

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

	protected:
		int EncodeFrame(EStreamType n_eStreamType, AVFrame* n_Frame);

	protected:
		bool m_bIsEnd[(int)EStreamType::ST_Size] = { true };

		std::map<EStreamType, FCodecContext>* m_OutputCodecContext = nullptr;
	};
}
