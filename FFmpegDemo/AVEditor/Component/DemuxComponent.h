#pragma once

namespace aveditor
{
	class CInputContext;
	class AVEDITOR_API CDemuxComponent : public IComponent
	{
	public:
		CDemuxComponent();
		virtual ~CDemuxComponent();

		void Init(int n_nContextIndex,
			const std::map<EStreamType, int64_t>& n_mPts);
		int Run();
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

		CInputContext* GetInputContext();

		const std::map<EStreamType, int64_t> GetPts() const;

		// Write null AVPacket/AVFrame
		void WriteEndData();

	protected:
		struct FStreamInfo
		{
			// Max pts of last input context
			int64_t LastMax = 0;
			// Max pts of current input context
			int64_t CurMax = 0;

			AVRational TimeBase = { 1,1 };

			std::vector<int64_t> Sections;

			int64_t IsPacketUseable(const int64_t n_nPts, const int64_t n_nDuration);
		};


		int		m_nContextIndex = -1;

		std::map<EStreamType, FStreamInfo> m_mStreamInfo;
	};
}
