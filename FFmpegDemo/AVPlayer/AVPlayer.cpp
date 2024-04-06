#include "AVPlayer.h"
#pragma comment(lib, "AVEditor.lib")

AVPlayer::AVPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(ui.btnPlay, &QPushButton::clicked,
		this, &AVPlayer::OnPlayClicked);
}

AVPlayer::~AVPlayer()
{
	Editor.Stop();
	Editor.Join();
}

void AVPlayer::Init()
{
	m_nWidth = ui.label->size().width();
	m_nHeight = ui.label->size().height();

	FFormatContext& Input = Editor.OpenInputFile("1.mp4");

	AVCodecContext* vCodec = Input.GetCodecContext(EStreamType::ST_Video);
	//AVCodecContext* aCodec = Input.GetCodecContext(EStreamType::EST_Audio);

	// Maybe AVFrame decoded from input context should be convert to target 
	// sample format/pixel format, so the target codec context should be created
	FFormatContext& Output = Editor.GetOutputContext()->GetContext();
	Output.BuildEncodeCodecContext(AVCodecID::AV_CODEC_ID_RAWVIDEO, vCodec);
	//Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_VIDEO));
	Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_AUDIO));
	// could not use this function, as the frame_size is 0
	//Output.BuildCodecContext(aCodec->codec_id, aCodec);

	AVCodecContext* ovCodec = Output.GetCodecContext(EStreamType::ST_Video);

	ovCodec->width = m_nWidth;
	ovCodec->height = m_nHeight;
	ovCodec->pix_fmt = GetSupportedPixelFormat(ovCodec->codec, 
		AVPixelFormat::AV_PIX_FMT_RGB24);

	AVCodecContext* aoCodec = Output.GetCodecContext(EStreamType::ST_Audio);
	aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S32;

	Editor.SetOutputIOHandle(this);
}

int AVPlayer::ReceiveData(const EStreamType n_eStreamType, void* n_Data, EDataType n_eType, int n_nIndex)
{
	if (!n_Data) 
		return 0;
	// play frame
	// ...
	DebugLog("Output frame %d, Type %d\n", n_eStreamType, n_eType);

	switch (n_eStreamType)
	{
	case EStreamType::ST_Video:
		VideoFrameArrived((AVFrame*)n_Data);
		break;
	case EStreamType::ST_Audio:
		break;
	}


	AVFreeData(n_eType, n_Data);

	return 0;
}

void AVPlayer::VideoFrameArrived(const AVFrame* n_Frame)
{
	QPixmap Pixmap;

	//Pixmap.loadFromData((const uchar*)n_Frame->data[0], n_Frame->linesize[0]);
	QImage img(n_Frame->data[0], 
		n_Frame->width, n_Frame->height, n_Frame->linesize[0], QImage::Format_RGB888);
	Pixmap = QPixmap::fromImage(img);
	if (!Pixmap.isNull())
	{
		ui.label->setPixmap(Pixmap);
	}

	//int delta = n_Frame->pts * 1000.0f / n_Frame->time_base.den;
	std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

void AVPlayer::AudioFrameArrived(const AVFrame* n_Frame)
{

}

void AVPlayer::OnPlayClicked()
{
	Editor.Stop();

	if (Editor.GetStatus() != EEditStatus::ES_Stopped)
	{
		return;
	}

	Init();

	Editor.Start();
}

void AVPlayer::OnStopClicked()
{
	if (Editor.GetStatus() == EEditStatus::ES_Running)
	{
		Editor.Stop();
	}
}
