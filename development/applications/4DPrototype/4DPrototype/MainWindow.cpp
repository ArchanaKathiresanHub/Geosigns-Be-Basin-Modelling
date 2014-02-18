#include "MainWindow.h"
#include "Mesh.h"
#include "BpaMesh.h"

#include <QtGui/QFileDialog>

void MainWindow::onActionOpenActivated()
{
  QString caption = "Open file";
  QString dir;
  QString filter = "Cauldron project (*.project3d)";
  QString filename = QFileDialog::getOpenFileName(this, "Open file", dir, filter);

  if(!filename.isNull())
  {
    foo(filename.toAscii().data());
    m_ui.widget->getViewer()->setSceneGraph(createOIVTree(filename.toAscii().data()));
  }
}

void MainWindow::connectSignals()
{
  connect(m_ui.action_Open, SIGNAL(activated()), this, SLOT(onActionOpenActivated()));
}

MainWindow::MainWindow()
{
	m_ui.setupUi(this);

	// Remove all the ugly buttons and scroll wheels that 
	// you always get for free with these OIV viewers
	m_ui.widget->setDecoration(false);
  m_ui.widget->getViewer()->setBackgroundColor(SbColor(.1f, .1f, .3f));

  connectSignals();
}