#pragma once
#include <QAudioOutput>
#include <QtWidgets/QWidget>
#include "ui_AVPlayer.h"
#include "../AVEditor/AVEditor.h"

using namespace aveditor;


class AVPlayer : public QWidget, public IAVIOHandle
{
    Q_OBJECT

public:
    AVPlayer(QWidget *parent = nullptr);
    ~AVPlayer();

	void Init();


	int ReceiveData(const EStreamType n_eStreamType,
		void* n_Data, EDataType n_eType, int n_nIndex);

	virtual void VideoFrameArrived(const AVFrame* n_Frame);
	virtual void AudioFrameArrived(const AVFrame* n_Frame);

protected:
	void SamplesRemainInBuffer(int n_nFree);

signals:
	void OnVideoArrived(const QPixmap n_Pixmap);

protected slots:
	void OnPlayClicked();
	void OnStopClicked();
	void slotVideoArrived(const QPixmap n_Pixmap);

private:
    Ui::AVPlayerClass ui;

	QAudioOutput*	m_AudioOutput = nullptr;
	QIODevice*		m_Device = nullptr;

	CEditor			Editor;

	// Is audio stream exists
	bool	m_bIsAudioExists = true;
	// Indicates audio/video stream arrived
	bool	m_bReady[2] = { false };
	// Timer stamp
	double	m_dTime = 0;
	// Duration of one Sample
	double	m_dDurionPerSample = 0;
	// How many samples in QAudioOutput buffer
	int		m_nSamplesInBuffer = 0;
	// How many bytes does a sample occupy
	int		m_nBytesPerSample = 1;
};
