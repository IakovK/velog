#include "Emulator.h"
#include <QDataStream>

Emulator::Emulator(int portNumber, QObject *parent) :
	QObject(parent),
	m_bConnected(false),
	m_state(init),
	m_pServer(nullptr),
	m_pSocket(nullptr),
	m_pPrevSocket(nullptr)
{
	m_pServer = new QTcpServer(this);

	// whenever a user connects, it will emit signal
	connect(m_pServer, &QTcpServer::newConnection,
		this, &Emulator::newConnection);

	if (!m_pServer->listen(QHostAddress::Any, portNumber))
	{
		qDebug() << "Server could not start";
	}
	else
	{
		qDebug() << "Server started!";
	}
}

void Emulator::newConnection()
{
	qDebug() << "Emulator::newConnection: m_bConnected = " << m_bConnected << ", m_state = " << m_state;
	QTcpSocket *socket = m_pServer->nextPendingConnection();
	if (m_bConnected)
	{
		socket->close();
		socket->deleteLater();
	}
	else
	{
		m_bConnected = true;
		connect(socket, &QAbstractSocket::disconnected, this, &Emulator::handleDisconnected);
		connect(socket, &QIODevice::readyRead, this, &Emulator::readData);
		connect(socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &Emulator::networkError);
		m_pSocket = socket;
		m_state = init;
	}
	if (m_pPrevSocket != nullptr)
		m_pPrevSocket->deleteLater();
}

void Emulator::readData()
{
	switch (m_state)
	{
	case init:
		ReadInitData();
		break;
	case active:
		ReadCommand();
		break;
	}
}

void CalculateStrobeInfo(quint8 *data, strobeInfo info, amp_strob_struct_t &result)
{
	int low_index = info.begin * 255;
	int high_index = std::min((int)((info.begin + info.duration) * 255), 256);
	int val = info.value;
	auto it = std::max_element(data + low_index, data + high_index);
	if (*it > val)
	{
		auto idx = std::distance(data, it);
		result.ampl = *it;
		result.time = idx;
	}
	else
	{
		result.ampl = 0;
		result.time = 0;
	}
}

void InitData(channelInfoEx &channel)
{
	if (channel.channelNum <= 4)
	{
		for (int j = 0; j <= 255; j++)
		{
			channel.data[j] = pow(j / 255., channel.channelNum) * 255;
		}
	}
	else
	{
		double Pi_2 = asin(1.);
		for (int j = 0; j <= 255; j++)
		{
			channel.data[j] = pow(sin(Pi_2 * j / 255.), channel.channelNum - 4) * 255;
		}
	}
	for (int j = 0; j < channel.numStrobes; j++)
	{
		CalculateStrobeInfo(channel.data, channel.strobes[j], channel.strobe[j]);
	}
}

void Emulator::ReadInitData()
{
	qint8 command;
	m_pSocket->read((char*)&command, 1);
	if (command != INIT_DEVICE)
	{
		DisconnectSocket();
		return;
	}

	qDebug() << "Emulator::ReadInitData: reading data: bytesAvailable = " << m_pSocket->bytesAvailable();
	QDataStream qds(m_pSocket);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

	qint8 numChannels;
	qds >> numChannels;
	m_numChannels = numChannels;
	qDebug() << "Emulator::ReadInitData: m_numChannels = " << m_numChannels;
	memset(channels, 0, sizeof channels);
	for (int j = 0; j < m_numChannels; j++)
	{
		qds >> channels[j].channelNum;
		qds >> channels[j].tact;
		qds >> channels[j].ampl;
		qds >> channels[j].begin;
		qds >> channels[j].end;
		qds >> channels[j].amplF;
		qds >> channels[j].TT;
		qds >> channels[j].TV;

		qint32 dm;
		qds >> dm;
		channels[j].detMode = (DetectionModeType)dm;

		qds >> channels[j].numStrobes;
		qDebug() << "Emulator::ReadInitData: channel = " << j << ", numStrobes = " << channels[j].numStrobes;
		for (int j1 = 0; j1 < channels[j].numStrobes; j1++)
		{
			qds >> channels[j].strobes[j1].value;
			qds >> channels[j].strobes[j1].begin;
			qds >> channels[j].strobes[j1].duration;
		}
		InitData(channels[j]);
	}
	m_state = active;
	qDebug() << "Emulator::ReadInitData: exit: bytesAvailable = " << m_pSocket->bytesAvailable();
	if (m_pSocket->bytesAvailable() != 0)
		readData();
}

void Emulator::ReturnAscan()
{
	qDebug() << "Emulator::ReturnAscan: entry. bytesAvailable = " << m_pSocket->bytesAvailable();
	QDataStream qds(m_pSocket);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);
	quint8 channelNum;
	qds >> channelNum;
	qDebug() << "Emulator::ReturnAscan: channelNum = " << channelNum << ", bytesAvailable = " << m_pSocket->bytesAvailable();
	for (int j = 0; j < m_numChannels; j++)
	{
		if (channels[j].channelNum == channelNum)
		{
			qds << REPLY_ASCAN;
			qds.writeBytes((const char*)channels[j].data, 256);
			break;
		}
	}
	qDebug() << "Emulator::ReturnAscan: exit. bytesAvailable = " << m_pSocket->bytesAvailable();
}

void Emulator::ReturnStrobe()
{
	qDebug() << "Emulator::ReturnStrobe: entry. bytesAvailable = " << m_pSocket->bytesAvailable();
	amp_struct_t data{ 0 };
	for (int j = 0; j < m_numChannels; j++)
	{
		auto &strobes = data.ampl_tact[channels[j].tact - 1].ampl_us[channels[j].ampl - 1];
		for (int j1 = 0; j1 < channels[j].numStrobes; j1++)
		{
			strobes.ampl[j1] = channels[j].strobe[j1];
		}
	}
	QDataStream qds(m_pSocket);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);
	qds << (quint8)REPLY_STROBE;
	for (int t = 0; t < NUM_TACTS; t++)
	{
		for (int u = 0; u < NUM_USS; u++)
		{
			for (int s = 0; s < NUM_RESULT_STRBS; s++)
			{
				qds << data.ampl_tact[t].ampl_us[u].ampl[s].time;
				qds << data.ampl_tact[t].ampl_us[u].ampl[s].ampl;
			}
		}
	}
	qDebug() << "Emulator::ReturnStrobe: exit. bytesAvailable = " << m_pSocket->bytesAvailable();
}

void Emulator::ReadCommand()
{
	qint8 command;
	m_pSocket->read((char*)&command, 1);
	switch (command)
	{
	case GET_ASCAN:
		qDebug() << "Emulator::ReadCommand: GET_ASCAN";
		ReturnAscan();
		break;
	case GET_STROBE:
		qDebug() << "Emulator::ReadCommand: GET_STROBE";
		ReturnStrobe();
		break;
	default:
		qDebug() << "Emulator::ReadCommand: unknown command";
		DisconnectSocket();
		break;
	}
	for (int j = 0; j < m_numChannels; j++)
	{
		std::rotate(channels[j].data, channels[j].data + 1, channels[j].data + 256);
		for (int j1 = 0; j1 < channels[j].numStrobes; j1++)
		{
			CalculateStrobeInfo(channels[j].data, channels[j].strobes[j1], channels[j].strobe[j1]);
		}
	}
	if (m_pSocket->bytesAvailable() != 0)
		readData();
}

void Emulator::networkError(QAbstractSocket::SocketError error)
{
	qDebug() << "network error: " << m_pSocket->errorString();
	DisconnectSocket();
}

void Emulator::handleDisconnected()
{
	qDebug() << "Emulator::handleDisconnected: state = " << m_pSocket->state();
	DisconnectSocket();
}

void Emulator::DisconnectSocket()
{
	if (m_pSocket != nullptr)
	{
		if (m_pSocket->state() != QAbstractSocket::UnconnectedState)
		{
			m_pSocket->close();
		}
		m_pPrevSocket = m_pSocket;
		m_pSocket = nullptr;
		m_bConnected = false;
	}
}
