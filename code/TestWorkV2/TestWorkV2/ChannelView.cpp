#include "ChannelView.h"

extern QColor levelColors[];

ChannelView::ChannelView(QWidget *parent)
	: QWidget(parent),
	m_bConnected(false),
	m_info(nullptr),
	m_data(nullptr)
{
	ui.setupUi(this);
	ui.plot->yAxis->setVisible(false);
	ui.plot->xAxis->setVisible(false);
	connect(ui.plot, &QCustomPlot::mouseRelease, this, &ChannelView::PlotClicked);
}

ChannelView::~ChannelView()
{
}

void ChannelView::setChannelName(QString name)
{
	ui.name->setText(name);
}

void ChannelView::PlotClicked(QMouseEvent *event)
{
	emit PlotSelected(m_info->channelNum);
}

void ChannelView::setChannelData(channelInfo *info, channelData *data)
{
	m_info = info;
	m_data = data;

	QVector<double> x(256);
	for (int i = 0; i < 256; ++i)
	{
		x[i] = i;
	}
	for (int j = 0; j < info->numStrobes; j++)
	{
		QVector<double> y(256);
		for (int i = 0; i < 256; ++i)
		{
			y[i] = m_info->strobes[j].value;
		}
		levelGraphs[j] = ui.plot->addGraph();
		levelGraphs[j]->setData(x, y);
		levelGraphs[j]->setLineStyle(QCPGraph::lsNone);
		levelGraphs[j]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, levelColors[j], levelColors[j], 1));
		levelGraphs[j]->setPen(levelColors[j]);
		levelGraphs[j]->setScatterSkip(3);
	}
	ui.plot->xAxis->setRange(0, 256);
	ui.plot->yAxis->setRange(0, 256);
	ui.plot->replot();
}

void ChannelView::deviceConnected()
{
	m_bConnected = true;
	for (int j = 0; j < m_info->numStrobes; j++)
	{
		dataGraphs[j] = ui.plot->addGraph();
		dataGraphs[j]->setPen(levelColors[j]);
	}
	ui.plot->replot();
}

void ChannelView::deviceDisconnected()
{
	m_bConnected = false;
	for (int j = 0; j < m_info->numStrobes; j++)
	{
		ui.plot->removeGraph(dataGraphs[j]);
	}
	ui.plot->replot();
}

void ChannelView::handleStrobeData()
{
	if (!m_bConnected)
		return;
	for (int j = 0; j < m_info->numStrobes; j++)
	{
		int x1 = m_info->strobes[j].begin * 256;
		int x2 = (m_info->strobes[j].begin + m_info->strobes[j].duration) * 256;
		QVector<double> x(x2 - x1);
		for (int i = 0; i < x.size(); ++i)
		{
			x[i] = i;
		}
		if (m_data->strobe[j].ampl != 0)
		{
			QVector<double> y(x.size());
			for (int i = 0; i < y.size(); ++i)
			{
				y[i] = m_data->strobe[j].ampl;
			}
			dataGraphs[j]->setData(x, y);
		}
		else
		{
			// clear data for the graph
			dataGraphs[j]->data()->clear();
		}
	}
	ui.plot->replot();
}
