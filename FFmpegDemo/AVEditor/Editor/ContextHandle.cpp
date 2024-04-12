#include "pch.h"
#include "ContextHandle.h"


namespace aveditor
{
	void IContextHandle::FillVideoFrame(AVFrame* n_Frame,
		const void* n_Data, const int& n_nSize) const
	{
		int nPlanes = GetVideoPlanes();

		if (1 == nPlanes)
		{
			memcpy_s(n_Frame->data[0], n_Frame->linesize[0],
				n_Data, n_nSize);
		}
	}

	void IContextHandle::FillAudioFrame(AVFrame* n_Frame,
		const void* n_Data, const int& n_nSize) const
	{
		int nIsPlanar = GetAudioPlanar();
		int nBytesPerSample = GetBytesPerSample();

		if (0 == nIsPlanar)
		{
			memcpy_s(n_Frame->data[0], n_Frame->linesize[0],
				n_Data, n_nSize);
		}
	}

	void IContextHandle::SetVideoPlanes(const int n_nPlanes)
	{
		m_nPlanes = n_nPlanes;
	}

	const int IContextHandle::GetVideoPlanes() const
	{
		return m_nPlanes;
	}

	void IContextHandle::SetAudioPlanar(const int n_nPlanar)
	{
		m_nIsPlanar = n_nPlanar;
	}

	const int IContextHandle::GetAudioPlanar() const
	{
		return m_nIsPlanar;
	}

	void IContextHandle::SetBytesPerSample(const int n_nBytes)
	{
		m_nBytesPerSample = n_nBytes;
	}

	const int IContextHandle::GetBytesPerSample() const
	{
		return m_nBytesPerSample;
	}

	void IContextHandle::SetChannelCount(const int n_nCount)
	{
		m_nChannelCount = n_nCount;
	}

	const int IContextHandle::GetChannelCount() const
	{
		return m_nChannelCount;
	}

	void IContextHandle::SetAVIOHandle(IAVIOHandle* n_AVIOHandle)
	{
		m_AVIOHandle = n_AVIOHandle;
	}

	int IContextHandle::WriteFrameData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (m_AVIOHandle)
			ret = m_AVIOHandle->ReceiveData(
				n_eStreamType, n_Data, n_eType, n_nIndex);

		return ret;
	}

}
