#pragma once

namespace aveditor
{
	class AVEDITOR_API CAudioMixComponent : public IComponent
	{
	public:
		CAudioMixComponent();
		~CAudioMixComponent();

		virtual void Init(int n_nBatchIndex);
		virtual int Run();
		virtual void Release();

		// Is the filter valid
		bool IsValid() const;

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
		int CreateFilter(const std::vector<int>& n_vContextIndex);

	protected:
		FFilter			m_Filter;
		AVFrame*		m_PopFrame = nullptr;

		AVCodecContext* m_OutputCodecContext = nullptr;
	};
}
