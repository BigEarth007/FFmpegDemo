#include "pch.h"
#include "Cale.h"



namespace aveditor
{
	FCale::~FCale()
	{
		Release();
	}

	SwsContext* FCale::Alloc(
		const int& n_nInputWidth, 
		const int& n_nInputHeight, 
		const AVPixelFormat& n_eInputPixelFormat, 
		const int& n_nOutputWidth, 
		const int& n_nOutputHeight, 
		const AVPixelFormat& n_eOutputPixelFormat)
	{
		Release();

		m_Context = sws_getContext(
			n_nInputWidth, n_nInputHeight, n_eInputPixelFormat,
			n_nOutputWidth, n_nOutputHeight, n_eOutputPixelFormat,
			SWS_BICUBIC, nullptr, nullptr, nullptr);

		ThrowExceptionExpr(!m_Context, "Fail to alloc sws context.\n");

		int ret = av_image_alloc(m_CaleData, m_LineSize,
			n_nOutputWidth, n_nOutputHeight, n_eOutputPixelFormat, 1);

		ThrowExceptionCodeExpr(ret < 0, ret, "Fail to allocate destination image");

		m_nInputWidth = n_nInputWidth;
		m_nInputHeight = n_nInputHeight;
		m_eInputPixelFormat = n_eInputPixelFormat;
		m_nOutputWidth = n_nOutputWidth;
		m_nOutputHeight = n_nOutputHeight;
		m_eOutputPixelFormat = n_eOutputPixelFormat;

		return m_Context;
	}

	SwsContext* FCale::Alloc(AVCodecContext* n_InputCodecContext, 
		AVCodecContext* n_OutputCodecContext)
	{
		return Alloc(
			n_InputCodecContext->width, 
			n_InputCodecContext->height, 
			n_InputCodecContext->pix_fmt,
			n_OutputCodecContext->width, 
			n_OutputCodecContext->height, 
			n_OutputCodecContext->pix_fmt);
	}

	void FCale::Release()
	{
		for (int i = 0;i < 4; i++)
		{
			if (m_CaleData[i])
				av_freep(&m_CaleData[i]);
		}
		
		memset(m_LineSize, 0, sizeof(m_LineSize));

		if (m_Context)
		{
			sws_freeContext(m_Context);
			m_Context = nullptr;
		}
	}

	int FCale::Cale(const uint8_t** n_InputData, const int* n_InputLineSize)
	{
		return Cale(n_InputData, n_InputLineSize, m_CaleData, m_LineSize);
	}

	int FCale::Cale(const uint8_t** n_InputData, const int* n_InputLineSize,
		uint8_t** n_OutputData, const int* n_OuputLineSize)
	{
		ThrowExceptionExpr(!m_Context || !m_CaleData[0], 
			"You should call function Alloc() first.\n");

		return sws_scale(m_Context, n_InputData, n_InputLineSize, 0, 
			m_nInputHeight, n_OutputData, n_OuputLineSize);
	}

	int FCale::Cale(const AVFrame* n_InputFrame)
	{
		return Cale((const uint8_t**)n_InputFrame->data, n_InputFrame->linesize);
	}

}
