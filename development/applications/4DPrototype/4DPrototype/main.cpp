#include "MainWindow.h"

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	return app.exec();
}