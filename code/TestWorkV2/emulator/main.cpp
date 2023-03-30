#include <QCoreApplication>
#include "Emulator.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc < 2)
	{
		qDebug() << "Usage: emulator <port number>" << "\n";
		return 0;
	}

	int portNumber = QString(argv[1]).toInt();
	qDebug() << "emulator: portNumber = " << portNumber << "\n";

	Emulator server(portNumber, &a);

	return a.exec();
}
