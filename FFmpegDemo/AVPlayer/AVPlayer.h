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

protected slots:
	void OnPlayClicked();
	void OnStopClicked();

private:
    Ui::AVPlayerClass ui;

	QAudioOutput* m_AudioOutput = nullptr;
	QIODevice* m_Device = nullptr;

	long m_nWidth = 0;
	long m_nHeight = 0;
	CEditor Editor;

	bool	m_bIsAudioExists = true;
	bool	m_bReady[2] = { false };
	// Timer stamp
	double	m_dTime = 0;
	int		m_nSamplesInBuffer = 0;
};
