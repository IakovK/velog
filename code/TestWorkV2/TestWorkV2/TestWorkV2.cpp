#include "TestWorkV2.h"
#include <qmessagebox>
#include <QHostAddress>
#include <QHostInfo>

QColor levelColors[]{ QColor(93, 129, 180), QColor(224, 155, 36), QColor(142, 176, 49),
QColor(235, 98, 53), QColor(134, 120, 178) };

TestWorkV2::TestWorkV2(QWidget *parent)
	: QMainWindow(parent)
	, m_curChannel(-1)
{
    ui.setupUi(this);

	initState();

	setupCharts();
	channelSelected(0);

	setControls();
	connectSignals();
}

void TestWorkV2::setupCharts()
{
	// init left half charts
	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		chw[j] = new ChannelView(ui.channelsList);;
		chw[j]->setChannelName(QString("CH%1").arg(j + 1));
		chw[j]->setChannelData(&channels[j], &data[j]);
		ui.verticalLayout_5->addWidget(chw[j]);
	}

	// setup strobe display chart
	ui.strobeDisplay->xAxis->setVisible(true);
	ui.strobeDisplay->xAxis->setRange(0, 256);
	ui.strobeDisplay->xAxis->setTicks(false);

	ui.strobeDisplay->yAxis->setVisible(true);
	ui.strobeDisplay->yAxis->setRange(0, 256);
	ui.strobeDisplay->yAxis->setTickLength(0);
	ui.strobeDisplay->yAxis->setSubTickLength(0);

	ui.strobeDisplay->xAxis2->setVisible(true);
	ui.strobeDisplay->xAxis2->setRange(0, 256);
	ui.strobeDisplay->xAxis2->setTicks(false);

	ui.strobeDisplay->yAxis2->setVisible(true);
	ui.strobeDisplay->yAxis2->setRange(0, 256);
	ui.strobeDisplay->yAxis2->setTickLength(0);
	ui.strobeDisplay->yAxis2->setSubTickLength(0);

	// setup a-scan display chart
	ui.ascanDisplay->xAxis->setVisible(true);
	ui.ascanDisplay->xAxis->setRange(0, 256);
	ui.ascanDisplay->xAxis->setLabel("мкс");

	ui.ascanDisplay->yAxis->setVisible(true);
	ui.ascanDisplay->yAxis->setRange(0, 256);
	ui.ascanDisplay->addGraph();
}

TestWorkV2::~TestWorkV2()
{
}

channelInfo channelDescr[]{
	{
		1, 1, 1, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		5, {{69, 0.891834, 0.811977}, {154, 0.148129, 0.187582}, {116,
	0.119056, 0.586505}, {182, 0.972847, 0.881379}, {170, 0.0179933,
	0.961698}}
	},
	{
		2, 2, 1, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		5, {{205, 0.302505, 0.670219}, {237, 0.278291,
	0.771418}, {222, 0.96063, 0.436057}, {50, 0.624055,
	0.174036}, {109, 0.563715, 0.654169}}
	},
	{
		3, 3, 1, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		4, {{111, 0.278748,
	0.439231}, {81, 0.814593, 0.0679227}, {131, 0.753833,
	0.591167}, {107, 0.968783, 0.599042}}
	},
	{
		4, 4, 1, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		5, {{162, 0.247399,
	0.61575}, {79, 0.0301714, 0.469568}, {168, 0.926062,
	0.581032}, {238, 0.715144, 0.713404}, {64, 0.647536,
	0.310266}}
	},
	{
		5, 1, 2, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		5, {{88, 0.418188, 0.919727}, {211, 0.900623,
	0.901395}, {63, 0.0923295, 0.0922335}, {237, 0.658914,
	0.487157}, {36, 0.264413, 0.204228}}
	},
	{
		6, 2, 2, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		2, {{141, 0.0193659,
	0.323156}, {231, 0.852646, 0.696675}}
	},
	{
		7, 3, 2, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		1, {{254, 0.661049,
	0.743745}}
	},
	{
		8, 4, 2, 0.5, 0.6, 0.7, 0.8, 0.9,
		DetectionModeType::PlusMinus,
		2, {{209, 0.359874, 0.582865}, {162, 0.443774,
	0.705929}}
	},
};

void TestWorkV2::initChannels()
{
	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		channels[j] = channelDescr[j];
	}
}

void TestWorkV2::initState()
{
	m_displayMode = Both;
	m_bConnected = false;
	m_connState = disconnected;
	initChannels();
}

void TestWorkV2::setControls()
{
	ui.displayMode->setCurrentIndex(m_displayMode);
	if (m_bConnected)
		ui.startStopBtn->setText(QString::fromUtf8("Стоп F3"));
	else
		ui.startStopBtn->setText(QString::fromUtf8("Пуск F3"));
	updateDisplay();
}

void TestWorkV2::setAscanVisible(bool visible)
{
	if (m_bConnected)
		ui.ascanWidget->setVisible(visible);
	else
		ui.ascanWidget->setVisible(false);
}

void TestWorkV2::updateDisplay()
{
	switch (m_displayMode)
	{
	case TestWorkV2::AScan:
		setAscanVisible(true);
		ui.strobeWidget->setVisible(false);
		break;
	case TestWorkV2::Strobe:
		setAscanVisible(false);
		ui.strobeWidget->setVisible(true);
		break;
	case TestWorkV2::Both:
		setAscanVisible(true);
		ui.strobeWidget->setVisible(true);
		break;
	default:
		break;
	}
}

void TestWorkV2::connectSignals()
{
	addAction(ui.actionConnect);
	connect(ui.displayMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TestWorkV2::onDisplayModeChanged);
	connect(ui.actionConnect, &QAction::triggered, this, &TestWorkV2::startStop);
	connect(ui.startStopBtn, &QPushButton::clicked, this, [=] { ui.actionConnect->trigger(); });

	connect(&tcpClient, &QAbstractSocket::connected, this, &TestWorkV2::handleConnected);
	connect(&tcpClient, &QAbstractSocket::disconnected, this, &TestWorkV2::handleDisconnected);
	connect(&tcpClient, &QIODevice::readyRead, this, &TestWorkV2::readData);
	connect(&tcpClient, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &TestWorkV2::networkError);
	connect(this, &TestWorkV2::strobeDataReady, this, &TestWorkV2::handleStrobeData);
	connect(this, &TestWorkV2::ascanDataReady, this, &TestWorkV2::handleAscanData);

	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		connect(chw[j], &ChannelView::PlotSelected, this, &TestWorkV2::channelSelected);
		connect(this, &TestWorkV2::deviceConnected, chw[j], &ChannelView::deviceConnected);
		connect(this, &TestWorkV2::deviceDisconnected, chw[j], &ChannelView::deviceDisconnected);
		connect(this, &TestWorkV2::strobeDataReady, chw[j], &ChannelView::handleStrobeData);
	}
}

void TestWorkV2::channelSelected(int channelNum)
{
	m_curChannel = indexByChannelNum(channelNum);
	updateStrobeDisplay();
	updateAScanDisplay();
	requestAscanInfo();
}

int TestWorkV2::indexByChannelNum(int channelNum)
{
	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		if (channels[j].channelNum == channelNum)
			return j;
	}
	return -1;
}

void TestWorkV2::handleStrobeData()
{
	if (!m_bConnected)
		return;
	if (m_curChannel < 0)
		return;

	auto &info = channels[m_curChannel];
	auto &cdata = data[m_curChannel];
	for (int j = 0; j < info.numStrobes; j++)
	{
		int x1 = info.strobes[j].begin * 256;
		int x2 = (info.strobes[j].begin + info.strobes[j].duration) * 256;
		QVector<double> x(x2 - x1);
		for (int i = 0; i < x.size(); ++i)
		{
			x[i] = i;
		}
		auto graph = ui.strobeDisplay->graph(info.numStrobes + j);
		if (graph != 0)
		{
			if (cdata.strobe[j].ampl != 0)
			{
				QVector<double> y(x.size());
				for (int i = 0; i < y.size(); ++i)
				{
					y[i] = cdata.strobe[j].ampl;
				}
				graph->setData(x, y);
			}
			else
			{
				// clear data for the graph
				graph->data()->clear();
			}
		}
	}
	ui.strobeDisplay->replot();
}

void TestWorkV2::handleAscanData()
{
	if (!m_bConnected)
		return;
	if (m_curChannel < 0)
		return;

	auto &cdata = data[m_curChannel];
	QVector<double> x(256);
	for (int i = 0; i < x.size(); ++i)
	{
		x[i] = i;
	}
	auto graph = ui.ascanDisplay->graph(0);
	if (graph != 0)
	{
		QVector<double> y(x.size());
		for (int i = 0; i < y.size(); ++i)
		{
			y[i] = cdata.data[i];
		}
		graph->setData(x, y);
	}
	ui.ascanDisplay->replot();
}

void TestWorkV2::updateStrobeDisplay()
{
	ui.strobeDisplay->clearGraphs();
	if (m_curChannel < 0)
		return;

	auto &info = channels[m_curChannel];
	QString caption = QString("Усилитель %1 Такт %2").arg(info.ampl).arg(info.tact);
	ui.strobeDisplayLabel->setText(caption);

	QVector<double> x(256);
	for (int i = 0; i < 256; ++i)
	{
		x[i] = i;
	}
	for (int j = 0; j < info.numStrobes; j++)
	{
		QVector<double> y(256);
		for (int i = 0; i < 256; ++i)
		{
			y[i] = info.strobes[j].value;
		}
		auto g = ui.strobeDisplay->addGraph();
		g->setData(x, y);
		g->setLineStyle(QCPGraph::lsNone);
		g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, levelColors[j], levelColors[j], 1));
		g->setPen(levelColors[j]);
		g->setScatterSkip(3);
	}

	// add graphs for strobe signals
	// graphs by index:
	// 0 <= i < info.numStrobes - strobe levels
	// info.numStrobes <= i < 2 * info.numStrobes - strobe data
	for (int j = 0; j < info.numStrobes; j++)
	{
		auto g = ui.strobeDisplay->addGraph();
		g->setPen(levelColors[j]);
	}
	ui.strobeDisplay->replot();
}

void TestWorkV2::clearStrobeDisplay()
{
	int numGraphs = ui.strobeDisplay->graphCount() / 2;
	for (int j = 0; j < numGraphs; j++)
	{
		auto g = ui.strobeDisplay->graph(numGraphs + j);
		if (g != nullptr)
			g->data()->clear();
	}
	ui.strobeDisplay->replot();
}

void TestWorkV2::updateAScanDisplay()
{
	if (m_curChannel < 0)
		return;

	auto &info = channels[m_curChannel];
	QString caption = QString("Усилитель %1 Такт %2").arg(info.ampl).arg(info.tact);
	ui.ascanDisplayLabel->setText(caption);
}

void TestWorkV2::onDisplayModeChanged(int num)
{
	m_displayMode = static_cast<DisplayModeType>(num);
	updateDisplay();
}

void TestWorkV2::startStop(bool)
{
	if (!m_bConnected)
		connectToDevice();
	else
		disconnectFromDevice();
}

void TestWorkV2::connectToDevice()
{
	QString addr = ui.devAddress->text();
	auto sl = addr.split(":");
	QString hostAddress = sl[0];
	int port = sl[1].toInt();
	m_connState = pending;
	tcpClient.connectToHost(hostAddress, port);
}

void TestWorkV2::disconnectFromDevice()
{
	tcpClient.disconnectFromHost();
}

void TestWorkV2::readData()
{
	qint8 command;
	tcpClient.read((char*)&command, 1);
	switch (command)
	{
	case REPLY_STROBE:
		readStrobeInfo();
		requestStrobeInfo();
		break;
	case REPLY_ASCAN:
		readAscanInfo();
		requestAscanInfo();
		break;
	}
	if (tcpClient.bytesAvailable() != 0)
		readData();
}

void TestWorkV2::readStrobeInfo()
{
	amp_struct_t as{ 0 };
	QDataStream qds(&tcpClient);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);
	for (int t = 0; t < NUM_TACTS; t++)
	{
		for (int u = 0; u < NUM_USS; u++)
		{
			for (int s = 0; s < NUM_RESULT_STRBS; s++)
			{
				qds >> as.ampl_tact[t].ampl_us[u].ampl[s].time;
				qds >> as.ampl_tact[t].ampl_us[u].ampl[s].ampl;
			}
		}
	}
	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		auto &strobes = as.ampl_tact[channels[j].tact - 1].ampl_us[channels[j].ampl - 1];
		for (int j1 = 0; j1 < channels[j].numStrobes; j1++)
		{
			data[j].strobe[j1] = strobes.ampl[j1];
		}
	}
	emit strobeDataReady();
}

void TestWorkV2::readAscanInfo()
{
	QDataStream qds(&tcpClient);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);
	if (m_curChannel < 0)
	{
		tcpClient.read(256);	// discard data
		return;
	}
	qds.readRawData((char*)data[m_curChannel].data, 256);	// read data into respective channel elt
	emit ascanDataReady();
}

void TestWorkV2::networkError(QAbstractSocket::SocketError error)
{
	tcpClient.disconnectFromHost();
	QMessageBox::information(this, "network error", tcpClient.errorString());
}

void TestWorkV2::handleConnected()
{
	if (m_connState == pending)
	{
		m_bConnected = true;
		setControls();
		m_connState = connected;
		sendInitData();
		readDevice();
		emit deviceConnected();
	}
}

void TestWorkV2::handleDisconnected()
{
	m_bConnected = false;
	m_connState = disconnected;
	setControls();
	clearStrobeDisplay();
	emit deviceDisconnected();
}

void TestWorkV2::sendInitData()
{
	QByteArray qba;
	QDataStream qds(&qba, QIODevice::WriteOnly);
	qds.setByteOrder(QDataStream::ByteOrder::BigEndian);
	qds.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);
	qds << (quint8)INIT_DEVICE;
	qds << (quint8)NUM_CHANNELS;
	for (int j = 0; j < NUM_CHANNELS; j++)
	{
		qds << channels[j].channelNum;
		qds << channels[j].tact;
		qds << channels[j].ampl;
		qds << channels[j].begin;
		qds << channels[j].end;
		qds << channels[j].amplF;
		qds << channels[j].TT;
		qds << channels[j].TV;
		qds << channels[j].detMode;
		qds << channels[j].numStrobes;
		for (int j1 = 0; j1 < channels[j].numStrobes; j1++)
		{
			qds << channels[j].strobes[j1].value;
			qds << channels[j].strobes[j1].begin;
			qds << channels[j].strobes[j1].duration;
		}
	}
	tcpClient.write(qba);
}

void TestWorkV2::readDevice()
{
	requestStrobeInfo();
	requestAscanInfo();
}

void TestWorkV2::requestStrobeInfo()
{
	QDataStream qds(&tcpClient);
	qds << (quint8)GET_STROBE;
}

void TestWorkV2::requestAscanInfo()
{
	if (ui.ascanWidget->isVisible() && m_curChannel >= 0)
	{
		QDataStream qds(&tcpClient);
		qds << (quint8)GET_ASCAN;
		qds << (quint8)channels[m_curChannel].channelNum;
	}
}

