#include <QAudioFormat>  
#include <QDebug>
#include "AVPlayer.h"
#pragma comment(lib, "AVEditor.lib")

using namespace aveditor;
constexpr auto knMaxBufferSize = 10240 * 2;

AVPlayer::AVPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(ui.btnPlay, &QPushButton::clicked,
		this, &AVPlayer::OnPlayClicked);

	connect(ui.btnStop, &QPushButton::clicked,
		this, &AVPlayer::OnStopClicked);

	connect(this, &AVPlayer::OnVideoArrived,
		this, &AVPlayer::slotVideoArrived);

	SetMediaFile("1.mp4");
	SetPlayView(ui.label);
}

AVPlayer::~AVPlayer()
{
	m_Editor.Stop();
	m_Editor.Join();
}

int AVPlayer::ReceiveData(const EStreamType n_eStreamType, void* n_Data, EDataType n_eType, int n_nIndex)
{
	// play frame
	// ...
	//DebugLog("Output frame %d, Type %d\n", n_eStreamType, n_eType);

	if (n_Data) m_nStreamMark |= (1 << (int)n_eStreamType);
	else m_nStreamMark &= ~(1 << (int)n_eStreamType);

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

void AVPlayer::SetMediaFile(const std::string& n_sMediaFile)
{
	m_sMediaFile = n_sMediaFile;
}

void AVPlayer::SetPlayView(QLabel* n_View)
{
	m_View = n_View;
}

void AVPlayer::Load()
{
	FFormatContext& Input = m_Editor.OpenInputFile(m_sMediaFile, ETask::T_Normal, kStreamVA);

	AVCodecContext* vCodec = Input.GetCodecContext(EStreamType::ST_Video);
	AVCodecContext* aCodec = Input.GetCodecContext(EStreamType::EST_Audio);

	// Maybe AVFrame decoded from input context should be convert to target 
	// sample format/pixel format, so the target codec context should be created
	FFormatContext& Output = m_Editor.GetOutputContext()->GetContext();

	if (vCodec)
	{
		Output.BuildEncodeCodecContext(AVCodecID::AV_CODEC_ID_RAWVIDEO, vCodec);
		AVCodecContext* ovCodec = Output.GetCodecContext(EStreamType::ST_Video);
		ovCodec->pix_fmt = GetSupportedPixelFormat(ovCodec->codec, 
			AVPixelFormat::AV_PIX_FMT_RGB24);
	}

	AVCodecContext* aoCodec = nullptr;
	if (aCodec)
	{
		Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_AUDIO));
		// could not use this function, as the frame_size is 0
		//Output.BuildCodecContext(aCodec->codec_id, aCodec);
		aoCodec = Output.GetCodecContext(EStreamType::ST_Audio);
		if (aoCodec) aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
	}

	if (aoCodec)
	{
		QAudioFormat format;
		format.setSampleRate(aoCodec->sample_rate);						// 例如：44.1 kHz  
		format.setChannelCount(aoCodec->ch_layout.nb_channels);			// 例如：立体声  
		int nFmt = GetBytesPerSample(aoCodec->sample_fmt);
		format.setSampleSize(nFmt * 8);									// 例如：16位样本  
		format.setCodec("audio/pcm");									// 例如：PCM编码  
		format.setByteOrder(QAudioFormat::LittleEndian);				// 字节序  
		format.setSampleType(QAudioFormat::SignedInt);					// 样本类型
		m_nBytesPerSample = nFmt * aoCodec->ch_layout.nb_channels;
		m_dDurionPerSample = 1.0 / aoCodec->sample_rate;

		if (!m_AudioOutput || m_AudioOutput->format() != format)
		{
			QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	
			m_AudioOutput = new QAudioOutput(info, format, this);
			m_AudioOutput->setBufferSize(knMaxBufferSize);
		}

		m_Device = m_AudioOutput->start();
	}

	m_Editor.SetFinishedCallback(
		[this]() {

			if (m_Device) m_Device->close();
			if (m_AudioOutput)
			{
				m_AudioOutput->stop();
				m_Device = nullptr;
			}
		});

	m_Editor.SetOutputIOHandle(this);
	m_Editor.SetMaxBufferSize(0);

	// for section select
	//m_Editor.AddSelectedSection(5, 10);
	//m_Editor.AddSelectedSection(22, 6);

	m_nSelectedStreams = m_Editor.GetOutputContext()->StreamsCode();
	m_nFreeBytes = knMaxBufferSize;
	m_dTime = 0;
}

void AVPlayer::VideoFrameArrived(const AVFrame* n_Frame)
{
	if (!n_Frame) return;

	double dTimestamp = n_Frame->pts * av_q2d(n_Frame->time_base);
	int offset = (dTimestamp - m_dTime) * 1000 * 1000;

	while (m_dTime == 0 && m_nStreamMark != m_nSelectedStreams)
	{
		// Wait audio stream coming
		std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
	}

	if ((m_nSelectedStreams & (1 << (int)EStreamType::ST_Audio)) == 0)
	{
		// The audio stream is not exists
		m_dTime = dTimestamp;
	}

	if (offset > 0)
		std::this_thread::sleep_for(std::chrono::microseconds(offset));

	QImage img(n_Frame->data[0],
		n_Frame->width, n_Frame->height, n_Frame->linesize[0], QImage::Format_RGB888);

	emit OnVideoArrived(QPixmap::fromImage(img));
}

void AVPlayer::AudioFrameArrived(const AVFrame* n_Frame)
{
	if (!m_AudioOutput) return;

	QAudio::State st = m_AudioOutput->state();
	if (m_Device &&
		(st == QAudio::State::IdleState || st == QAudio::State::ActiveState))
	{
		int nFree = m_AudioOutput->bytesFree();

		if (n_Frame)
		{
			while (m_dTime == 0 && m_nStreamMark != m_nSelectedStreams)
			{
				// Wait video stream coming
				std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
			}

			while (nFree < n_Frame->nb_samples * m_nBytesPerSample)
			{
				nFree = m_AudioOutput->bytesFree();
				std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDelay));
			}

			UpdateTime(nFree);

			m_Device->write(reinterpret_cast<const char*>(n_Frame->data[0]), n_Frame->linesize[0]);
			m_nFreeBytes = nFree - n_Frame->nb_samples * m_nBytesPerSample;
		}
		else
		{
			int nSamples = (knMaxBufferSize - n_nFree) / m_nBytesPerSample;
			double dTime = nSamples * m_dDurionPerSample;
			
			std::this_thread::sleep_for(std::chrono::milliseconds(int(dTime * 1000)));
			m_dTime += dTime;
		}
	}
}

void AVPlayer::UpdateTime(int n_nFree)
{
	int nSamplesCost = (n_nFree - m_nFreeBytes) / m_nBytesPerSample;
	m_dTime += nSamplesCost * m_dDurionPerSample;
}

void AVPlayer::OnPlayClicked()
{
	if (m_sMediaFile.empty()) return;

	try
	{
		if (m_Editor.GetStatus() == EEditStatus::ES_Stopped)
			Load();

		m_Editor.Start();
		
		//double dLength = m_Editor.GetInputContext()->Length();
	}
	catch (const std::exception& e)
	{
		qDebug() << e.what();
	}
}

void AVPlayer::OnStopClicked()
{
	m_Editor.Stop();
}

void AVPlayer::slotVideoArrived(const QPixmap n_Pixmap)
{
	if (!n_Pixmap.isNull() && m_View)
	{
		QPixmap Pixmap = n_Pixmap.scaled(m_View->size());
		m_View->setPixmap(Pixmap);
	}
}
