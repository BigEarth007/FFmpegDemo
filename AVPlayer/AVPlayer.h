#pragma once
#include <QAudioOutput>
#include <QtWidgets/QWidget>
#include "ui_AVPlayer.h"
#include "AVStudio.h"


class AVPlayer : public QWidget, public avstudio::IIOHandle
{
    Q_OBJECT

public:
    AVPlayer(QWidget *parent = nullptr);
    ~AVPlayer();

	// Receive video/audio frames that have been decoded
	int ReceiveData(const AVMediaType n_eMediaType,
		avstudio::EDataType n_eType, void* n_Data) override;

	size_t GetBufferSize(const AVMediaType n_eMediaType) const override;

	// Set file to play
	void SetMediaFile(const std::string& n_sMediaFile);

	// Set view to show video image
	void SetPlayView(QLabel* n_View);

protected:
	// Load media file, return length of file
	void Load();
	// Get video frame
	virtual void VideoFrameArrived(const AVFrame* n_Frame);
	// Get audio frame
	virtual void AudioFrameArrived(const AVFrame* n_Frame);

	void PlayVideo();
	void PlayAudio();

	// Update playing time stamp
	// int n_nFree: bytes available in QAudioOutput buffer
	void UpdateTime(int n_nFree);

signals:
	void OnVideoArrived(const QPixmap n_Pixmap);

protected slots:
	void OnPlayClicked();
	void OnStopClicked();
	void slotVideoArrived(const QPixmap n_Pixmap);

private:
    Ui::AVPlayerClass ui;

	QAudioOutput*		m_AudioOutput = nullptr;
	QIODevice*		m_Device = nullptr;

	// The file to play
	std::string		m_sMediaFile;
	// The view to show video image
	QLabel*			m_View = nullptr;

	avstudio::CEditor	m_Editor;

	// Indicates weather audio/video stream arrived
	int			m_nStreamMark = 0;
	// The selected streams of input context
	int			m_nSelectedStreams = 0;

	// Timer stamp
	double		m_dTime = 0;
	// Duration of one Sample
	double		m_dDurionPerSample = 0;
	// Number of free bytes in QAudioOutput buffer
	int			m_nFreeBytes = 0;
	// How many bytes does a sample occupy
	int			m_nBytesPerSample = 1;
	
	std::thread tVideo;
	std::thread tAudio;

	std::queue<AVFrame*> qVideo;
	std::queue<AVFrame*> qAudio;
};
