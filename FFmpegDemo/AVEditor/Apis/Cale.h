#pragma once


namespace aveditor
{
	struct AVEDITOR_API FCale
	{
		FCale() = default;
		~FCale();

		// Alloc sws context
		SwsContext* Alloc(
			const int& n_nInputWidth,
			const int& n_nInputHeight,
			const AVPixelFormat& n_eInputPixelFormat,
			const int& n_nOutputWidth,
			const int& n_nOutputHeight,
			const AVPixelFormat& n_eOutputPixelFormat);

		SwsContext* Alloc(AVCodecContext* n_InputCodecContext,
			AVCodecContext* n_OutputCodecContext);

		// Release 
		void Release();

		// Scale video data
		// return          the height of the output slice
		int Cale(const uint8_t** n_InputData, const int* n_InputLineSize);
		int Cale(const uint8_t** n_InputData, const int* n_InputLineSize,
			uint8_t** n_OutputData, const int* n_OuputLineSize);
		int Cale(const AVFrame* n_InputFrame);

		// Get parameters
		int				GetInputWidth()			{ return m_nInputWidth; }
		int				GetInputHeight()		{ return m_nInputHeight; }
		AVPixelFormat	GetInputPixelFormat()	{ return m_eInputPixelFormat; }
		int				GetOutputWidth()		{ return m_nOutputWidth; }
		int				GetOutputHeight()		{ return m_nOutputHeight; }
		AVPixelFormat	GetOutputPixelFormat()	{ return m_eOutputPixelFormat; }

		SwsContext*		m_Context = nullptr;

		// Output data when scale video frame data
		uint8_t*		m_CaleData[4] = {nullptr};
		int				m_LineSize[4] = { 0 };

	protected:
		int				m_nInputWidth			= 0;
		int				m_nInputHeight			= 0;
		AVPixelFormat	m_eInputPixelFormat		= AVPixelFormat::AV_PIX_FMT_NONE;
		int				m_nOutputWidth			= 0;
		int				m_nOutputHeight			= 0;
		AVPixelFormat	m_eOutputPixelFormat	= AVPixelFormat::AV_PIX_FMT_NONE;
	};
}
