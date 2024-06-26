#pragma once


namespace aveditor
{
	class AVEDITOR_API IContextHandle
	{
	public:
		// Fill video frame when write PCM data
		virtual void FillVideoFrame(AVFrame* n_Frame,
			const void* n_Data, const int& n_nSize) const;

		// Fill audio frame when write PCM data
		virtual void FillAudioFrame(AVFrame* n_Frame,
			const void* n_Data, const int& n_nSize) const;

		void SetVideoPlanes(const int n_nPlanes);
		const int GetVideoPlanes() const;

		void SetBytesPerPixel(const int n_nBytes);
		const int GetBytesPerPixel() const;

		void SetAudioPlanar(const int n_nPlanar);
		const int GetAudioPlanar() const;

		void SetBytesPerSample(const int n_nBytes);
		const int GetBytesPerSample() const;

		void SetChannelCount(const int n_nCount);
		const int GetChannelCount() const;

		void SetAVIOHandle(IAVIOHandle* n_AVIOHandle);

		int WriteFrameData(const EStreamType n_eStreamType,
			void* n_Data, EDataType n_eType, int n_nIndex = 0);

	protected:
		IAVIOHandle* m_AVIOHandle = nullptr;

		// For video frame
		int m_nPlanes = 0;
		int m_nBytesPerPixel = 0;

		// For audio frame
		int m_nIsPlanar = 0;
		int m_nBytesPerSample = 0;
		int m_nChannelCount = 0;
	};
}
