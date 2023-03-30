#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include "..\common\Protocol.h"

struct channelInfoEx : channelInfo
{
	quint8 data[256];
	amp_strob_struct_t strobe[NUM_RESULT_STRBS];
};

class Emulator : public QObject
{
	Q_OBJECT
public:
	explicit Emulator(int portNumber, QObject *parent = 0);

signals:

public slots:
	void newConnection();
	void readData();
	void networkError(QAbstractSocket::SocketError error);
	void handleDisconnected();

private:
	QTcpServer *m_pServer;
	QTcpSocket *m_pSocket;
	QTcpSocket *m_pPrevSocket;
	bool m_bConnected;
	enum State
	{
		init = 0,
		active
	};
	State m_state;
	int m_numChannels;
	channelInfoEx channels[NUM_CHANNELS];

	void DisconnectSocket();
	void ReadInitData();
	void ReadCommand();
	void ReturnAscan();
	void ReturnStrobe();
};
