#pragma once

namespace aveditor
{
	class IFrameConvert;
	class AVEDITOR_API CAVConverter
	{
	public:
		CAVConverter();
		~CAVConverter();

		void Init(FCodecContext* n_InputCodec,
			FCodecContext* n_OutputCodec,
			CEditor* n_Editor);
		void Release();

		int Coverting(const EDataType n_eDataType, void* n_Data);

		/*
		* Do with n_Frame
		* return value
		*	0: nothing to do
		*	1: convert the n_Frame
		*/
		int ConvertFrame(AVFrame* n_Frame);

		// Decoding Packet
		int Decoding(AVPacket* n_Packet);

		// If converter is valid or not
		bool IsConvertValid();

		// Set subnumber
		void SetSubnumber(const int n_nSubnumber);
		// Get subnumber
		const int GetSubnumber() const;

		// Is the packet should be decoded
		void SetDecodeFlag(const bool n_bDecode);
		// Is the packet should be decoded
		const bool GetDecodeFlag() const;

		// Set finished callback
		void SetFinishedCallback(CompCallback n_Callback);

		const int64_t GetFramePts() const;

	protected:
		// Compare codec between input and output context
		int CompCodecFormat();

		// Convert frame to target frame
		virtual int FinishedConvert(AVFrame* n_Frame);

	protected:
		CEditor*		m_Editor = nullptr;
		// Notify callback after decoding and converting
		CompCallback	m_Callback = nullptr;

		FCodecContext*	m_InputCodec = nullptr;
		FCodecContext*	m_OutputCodec = nullptr;

		IFrameConvert*	m_FrameConvert = nullptr;

		EStreamType		m_eStreamType = EStreamType::ST_Video;

		// Subnumber of input context
		int				m_nSubnumber = 0;

		// Is the packet should be decoded
		bool			m_bNeedDecode = false;
	};

	//////////////////////////////////////////////////////////////////////////
	class AVEDITOR_API CDecodeComponent : public IComponent
	{
	public:
		CDecodeComponent();
		virtual ~CDecodeComponent();

		void Init(int n_nContextIndex);
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

		// Finished decoding 
		int FinishedDecode(const EStreamType n_eStreamType,
			void* n_Data, EDataType n_eType, int n_nIndex = 0);

	protected:
		// decoder
		CAVConverter	m_Decoder[(int)EStreamType::ST_Size];
	};
}
