#pragma once

namespace aveditor
{
	class CEditor;
	class AVEDITOR_API CAVObject
	{
	public:
		CAVObject();
		~CAVObject();

		void Init();
		void Release();

		int RunDemux();
		int RunDecode(EStreamType n_eStreamType);
		int RunFilter();
		int RunEncode(EStreamType n_eStreamType);
		int RunMuxer();

		// Set Editor object
		void SetEditor(CEditor* n_Editor);
		// Get Editor object
		const CEditor* GetEditor() const;

		// start by batch
		void StartBatch(int n_nBatchIndex);

		// Is current batch end
		const bool IsBatchEnd() const;

		// Set all component to end
		void SetEndFlag();

		// Set max buffer queue size
		void SetMaxBufferSize(int n_nSize);

		// Set Audio/Video IO handle for output 
		void SetOutputIOHandle(IAVIOHandle* n_Handle);

	protected:
		// Reset input and audio mix components, so it can read next batch contexts
		void ResetComponents();

		// Calculate duration of last batch
		double Duration();

	protected:
		CEditor*						m_Editor = nullptr;

		std::vector<CDemuxComponent*>	m_vDemuxComp;
		std::vector<CDecodeComponent*>	m_vDecodeComp;

		CAudioMixComponent				m_AudioMixComp;

		CEncodeComponent				m_EncodeComp;
		CMuxerComponent					m_MuxerComp;

		// Max buffer size
		int								m_nMaxBufferSize = 80;
	};

}