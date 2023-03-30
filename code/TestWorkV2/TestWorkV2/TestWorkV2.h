#pragma once

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include "..\common\Protocol.h"
#include "ChannelView.h"
#include "ui_TestWorkV2.h"

class TestWorkV2 : public QMainWindow
{
    Q_OBJECT

public:
    TestWorkV2(QWidget *parent = nullptr);
	~TestWorkV2();

	enum DisplayModeType
	{
		AScan = 0,
		Strobe,
		Both
	};

	enum connectionState
	{
		pending,
		disconnected,
		connected
	};

private:
    Ui::TestWorkV2Class ui;

	DisplayModeType m_displayMode;
	bool m_bConnected;
	QTcpSocket tcpClient;
	connectionState m_connState;
	channelInfo channels[NUM_CHANNELS];
	channelData data[NUM_CHANNELS];
	ChannelView *chw[NUM_CHANNELS];
	int m_curChannel;	// channel selected in the left pane list

	void initState();
	void setupCharts();
	void initChannels();
	void setControls();
	void updateDisplay();
	void connectSignals();
	void connectToDevice();
	void disconnectFromDevice();
	void sendInitData();
	void readStrobeInfo();
	void readAscanInfo();
	void updateStrobeDisplay();
	void updateAScanDisplay();
	int indexByChannelNum(int channelNum);
	void clearStrobeDisplay();
	void setAscanVisible(bool visible);
	void readDevice();
	void requestStrobeInfo();
	void requestAscanInfo();

signals:
	void deviceConnected();
	void deviceDisconnected();
	void strobeDataReady();
	void ascanDataReady();

public slots:
	void onDisplayModeChanged(int num);
	void startStop(bool);
	void readData();
	void networkError(QAbstractSocket::SocketError error);
	void handleConnected();
	void handleDisconnected();
	void channelSelected(int channelNum);
	void handleStrobeData();
	void handleAscanData();
};
