#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.showMaximized();
	return a.exec();
}
