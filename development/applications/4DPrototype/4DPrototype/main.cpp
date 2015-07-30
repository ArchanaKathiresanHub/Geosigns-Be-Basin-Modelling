#include "MainWindow.h"

#include <Interface/ObjectFactory.h>
#include <Interface/ProjectHandle.h>

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	return app.exec();
}