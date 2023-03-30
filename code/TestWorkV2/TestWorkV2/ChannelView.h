#pragma once

#include <QWidget>
#include "ui_ChannelView.h"
#include "..\common\Protocol.h"

class ChannelView : public QWidget
{
	Q_OBJECT

public:
	ChannelView(QWidget *parent = nullptr);
	~ChannelView();
	void setChannelName(QString name);
	void setChannelData(channelInfo *info, channelData *data);

public slots:
	void PlotClicked(QMouseEvent *event);
	void deviceConnected();
	void deviceDisconnected();
	void handleStrobeData();

signals:
	void PlotSelected(int channelNum);

private:
	Ui::ChannelViewClass ui;

	channelInfo *m_info;
	channelData *m_data;
	QCPGraph *levelGraphs[NUM_RESULT_STRBS];	// show strobe levels
	QCPGraph *dataGraphs[NUM_RESULT_STRBS];		// show strobe data as received from device or emulator
	bool m_bConnected;							// true when connected to the device
};
