#include <QAudioFormat>  
#include <QDebug>
#include "AVPlayer.h"
#pragma comment(lib, "AVEditor.lib")

constexpr auto knMaxBufferSize = 10240 * 2;

AVPlayer::AVPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(ui.btnPlay, &QPushButton::clicked,
		this, &AVPlayer::OnPlayClicked);

	connect(ui.btnStop, &QPushButton::clicked,
		this, &AVPlayer::OnStopClicked);
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

	try
	{
		FFormatContext& Input = Editor.OpenInputFile("1.mp4", ETask::T_Normal, kStreamVA);

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
		aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;

		Editor.SetOutputIOHandle(this);
		Editor.SetMaxBufferSize(0);

		Editor.SetFinishedCallback(
			[this]() {

				m_Device->close();
				if (m_AudioOutput)
				{
					m_AudioOutput->stop();
					//m_AudioOutput->deleteLater();
				}
			});

		//m_bIsAudioExists = false;
		if (m_AudioOutput == nullptr)
		{
			QAudioFormat format;
			format.setSampleRate(aoCodec->sample_rate);						// 例如：44.1 kHz  
			format.setChannelCount(aoCodec->ch_layout.nb_channels);			// 例如：立体声  
			int nFmt = GetBytesPerSample(aoCodec->sample_fmt);
			format.setSampleSize(nFmt * 8);   // 例如：16位样本  
			format.setCodec("audio/pcm");									// 例如：PCM编码  
			format.setByteOrder(QAudioFormat::LittleEndian); // 字节序  
			format.setSampleType(QAudioFormat::SignedInt);   // 样本类型


			QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
			qDebug() << info.deviceName();

			if (!info.isFormatSupported(format))
			{
				qDebug() << "not support";
			}

			m_AudioOutput = new QAudioOutput(info, format, this);
			m_AudioOutput->setBufferSize(knMaxBufferSize);
		}
	}
	catch (const std::exception& e)
	{
		qDebug() << e.what();
	}

}

int AVPlayer::ReceiveData(const EStreamType n_eStreamType, void* n_Data, EDataType n_eType, int n_nIndex)
{
	if (!n_Data)
	{
		return 0;
	}
	// play frame
	// ...
	//DebugLog("Output frame %d, Type %d\n", n_eStreamType, n_eType);

	switch (n_eStreamType)
	{
	case EStreamType::ST_Video:
		VideoFrameArrived((AVFrame*)n_Data);
		break;
	case EStreamType::ST_Audio:
		AudioFrameArrived((AVFrame*)n_Data);
		break;
	}


	AVFreeData(n_eType, n_Data);

	return 0;
}

void AVPlayer::VideoFrameArrived(const AVFrame* n_Frame)
{
	double dTimestamp = n_Frame->pts * av_q2d(n_Frame->time_base);
	int offset = (dTimestamp - m_dTime) * 1000 * 1000;

	if (m_bIsAudioExists)
	{
		m_bReady[0] = true;
		while (!m_bReady[1])
		{
			// Wait audio stream coming
			std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
		}
	}
	else
	{
		m_dTime = dTimestamp;
	}

	if (offset > 0)
		std::this_thread::sleep_for(std::chrono::microseconds(offset));

	QPixmap Pixmap;

	QImage img(n_Frame->data[0], 
		n_Frame->width, n_Frame->height, n_Frame->linesize[0], QImage::Format_RGB888);
	Pixmap = QPixmap::fromImage(img);
	if (!Pixmap.isNull())
	{
		ui.label->setPixmap(Pixmap);
	}
}

void AVPlayer::AudioFrameArrived(const AVFrame* n_Frame)
{
	m_bReady[1] = true;
	while (!m_bReady[0])
	{
		// Wait video stream coming
		std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
	}

	QAudio::State st = m_AudioOutput->state();
	if (m_Device &&
		(st == QAudio::State::IdleState || st == QAudio::State::ActiveState))
	{
		int nFree = m_AudioOutput->bytesFree();

		while (nFree < n_Frame->nb_samples * 4)
		{
			nFree = m_AudioOutput->bytesFree();
			std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
		}

		int nRemain = knMaxBufferSize - nFree;
		int nCost = m_nSamplesInBuffer - nRemain;

		m_dTime += (double)nCost / 4 / n_Frame->sample_rate;

		//qDebug() << "Free: " << nFree << " Remain: " << nRemain << " Cost: " << nCost << " Wrote: " << m_nSamplesWrote;

		m_Device->write(reinterpret_cast<const char*>(n_Frame->data[0]), n_Frame->linesize[0]);
		m_nSamplesInBuffer = nRemain + n_Frame->nb_samples * 4;
	}
}

void AVPlayer::OnPlayClicked()
{
	Editor.Stop();

	if (Editor.GetStatus() == EEditStatus::ES_Stopped)
	{
		Init();

		m_dTime = 0;
		if (m_AudioOutput) m_Device = m_AudioOutput->start();
		Editor.Start();
		//m_Device->open(QIODevice::ReadWrite);
	}
}

void AVPlayer::OnStopClicked()
{
	if (Editor.GetStatus() == EEditStatus::ES_Running)
	{
		Editor.Stop();
	}
}
