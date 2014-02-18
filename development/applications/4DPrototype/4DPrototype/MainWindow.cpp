#include "MainWindow.h"
#include "Mesh.h"
#include "BpaMesh.h"

#include <Inventor/nodes/SoSwitch.h>

#include <QtGui/QFileDialog>

void MainWindow::onActionOpenTriggered()
{
  QString caption = "Open file";
  QString dir;
  QString filter = "Cauldron project (*.project3d)";
  QString filename = QFileDialog::getOpenFileName(this, "Open file", dir, filter);

  if(!filename.isNull())
  {
    SoSwitch* snapshotSwitch = (SoSwitch*)createOIVTree(filename.toAscii().data());
    m_ui.widget->getViewer()->setSceneGraph(snapshotSwitch);
    m_ui.snapshotSlider->setMinimum(0);
    m_ui.snapshotSlider->setMaximum(snapshotSwitch->getNumChildren() - 1);
  }
}

void MainWindow::onSliderValueChanged(int value)
{
  SoSwitch* snapshotSwitch = (SoSwitch*)m_ui.widget->getViewer()->getSceneGraph();
  snapshotSwitch->whichChild = value;
}

void MainWindow::connectSignals()
{
  connect(m_ui.action_Open, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
  connect(m_ui.snapshotSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
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