#include "MainWindow.h"

#include <MeshVizInterface/mapping/MoMeshviz.h>

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
	MoMeshViz::init();

	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	int result = app.exec();

	MoMeshViz::finish();

	return result;
}