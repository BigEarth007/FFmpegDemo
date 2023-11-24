#include "pch.h"
#include "AudioFifo.h"


namespace aveditor
{
	FAudioFifo::~FAudioFifo()
	{
		Release();
	}

	AVAudioFifo* FAudioFifo::Alloc(const AVSampleFormat& n_eSampleFormat,
		const int& n_nChannels, const int& n_nSamples)
	{
		m_eSampleFormat = n_eSampleFormat;
		m_nChannels = n_nChannels;
		m_nSamples = n_nSamples;

		m_Context = av_audio_fifo_alloc(m_eSampleFormat, m_nChannels, 1);

		ThrowExceptionExpr(!m_Context,
			"Fail to create audio FIFO buffer.\n");

		return m_Context;
	}

	void FAudioFifo::Release()
	{
		if (m_Context)
		{
			av_audio_fifo_free(m_Context);
			m_Context = nullptr;
		}
	}

	int FAudioFifo::Size()
	{
		return m_Context ? av_audio_fifo_size(m_Context) : 0;
	}

	int FAudioFifo::OutputFrameSize()
	{
		return m_nSamples;
	}

	int FAudioFifo::NextSampleCount()
	{
		int nSize = Size();
		if (nSize > m_nSamples)
			nSize = m_nSamples;

		return nSize;
	}

	void FAudioFifo::Push(uint8_t** n_Samples, const int& n_nSize)
	{
		int ret = 0;
		int nSize = Size();

		if (av_audio_fifo_space(m_Context) < n_nSize)
		{
			ret = av_audio_fifo_realloc(m_Context, nSize + n_nSize);
			ThrowExceptionCodeExpr(ret < 0, ret, "Fail to realloc FIFO buffer.\n");
		}

		if (n_nSize == 0) return;

		ret = av_audio_fifo_write(m_Context, (void**)n_Samples, n_nSize);
		ThrowExceptionCodeExpr(ret < n_nSize, ret, 
			"Fail to write audio samples into FIFO buffer.\n");
	}

	void FAudioFifo::Pop(uint8_t** n_Samples, const int& n_nSize)
	{
		int ret = av_audio_fifo_read(m_Context, (void**)n_Samples, n_nSize);
		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to read from fifo buffer.\n");
	}

	void FAudioFifo::Pop(AVFrame* n_Frame)
	{
		Pop(n_Frame->data, n_Frame->nb_samples);
	}

	bool FAudioFifo::IsReadable()
	{
		int nSize = Size();

		return nSize > 0 && m_nSamples <= nSize;
	}

}