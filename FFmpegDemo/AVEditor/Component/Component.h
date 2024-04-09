#pragma once

namespace aveditor
{
	/*
	* const EStreamType: stream type of data
	* void*: data, must be one of AVPacket/AVFrame
	* EDataType: data type
	*		DT_Packet: AVPacket
	*		DT_Frame: AVFrame
	* int: index of current batch 
	*		all input file will be executed by batch
	*		and one batch may contains more than one input file
	*/
	typedef std::function<int(const EStreamType, void*, EDataType, int)> CompCallback;

	class CEditor;
	class AVEDITOR_API IComponent : public IAVIOHandle
	{
	public:
		virtual ~IComponent();

		virtual void Init();
		virtual void Init(int n_nContextIndex);
		virtual int Run();
		virtual int Run(EStreamType n_eStreamType);
		virtual void Release();

		void ReleaseBuffer();

		// Write data into IO handle
		int WriteData(const EStreamType n_eStreamType,
			void* n_Data, EDataType n_eType, int n_nIndex = 0);

		// Set Editor object
		void SetEditor(CEditor* n_Editor);
		// Get Editor object
		const CEditor* GetEditor() const;

		// Set component ID
		void SetCompID(const ECompID& n_eCompID);
		// Get component ID
		const ECompID GetCompID() const;

		// Set end flag
		void SetEndFlag(const bool n_bEndFlag);
		// Get end flag, is it end now
		const bool GetEndFlag() const;

		// Set Audio/Video IO handle
		void SetIOHandle(IAVIOHandle* n_Handle);

		// Set callback for writing AVPacket* or AVFrame*
		void SetCompCallback(CompCallback func);

		// Force to stop
		void ForceStop();

	protected:
		virtual bool LimitBufferSize();

		enum class ECompStatus
		{
			CS_Normal = 0,
			CS_Ready,
			CS_Stop,
			CS_ForceStop,
		};

	protected:
		CEditor*		m_Editor = nullptr;
		// Callback for sending AVPacket* or AVFrame*
		CompCallback	m_Callback = nullptr;

		IAVIOHandle*	m_AVIOHandle = nullptr;

		// Output Context
		FFormatContext* m_OutputContext = nullptr;

		// Component ID
		ECompID			m_eCompID = ECompID::EI_Demux;

		// Is end of the task?
		ECompStatus		m_eStatus = ECompStatus::CS_Normal;
	};

}