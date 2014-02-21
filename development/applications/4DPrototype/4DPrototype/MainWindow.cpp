#include "MainWindow.h"
#include "SceneGraph.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"

#include <QtGui/QFileDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QLabel>

namespace di = DataAccess::Interface;

namespace
{
  const int TreeWidgetItem_FormationType = QTreeWidgetItem::UserType + 1;
  const int TreeWidgetItem_SurfaceType   = QTreeWidgetItem::UserType + 2;
  const int TreeWidgetItem_PropertyType  = QTreeWidgetItem::UserType + 3;
}

void MainWindow::fpsCallback(float fps, void* userData, SoQtViewer* viewer)
{
  MainWindow* mainWnd = reinterpret_cast<MainWindow*>(userData);
  mainWnd->onFps(fps);
}

void MainWindow::onFps(float fps)
{
  m_fpsLabel->setText(QString("%1 fps").arg(fps));
}

void MainWindow::loadProject(const QString& filename)
{
  closeProject();

  m_projectHandle = di::OpenCauldronProject(filename.toStdString(), "r");

  m_sceneGraph.reset(new SceneGraph(m_projectHandle));

  m_ui.widget->getViewer()->setSceneGraph(m_sceneGraph->root());
  m_ui.snapshotSlider->setMinimum(0);
  m_ui.snapshotSlider->setMaximum(m_sceneGraph->snapshotCount() - 1);

  updateUI();
}

void MainWindow::closeProject()
{
  if(m_projectHandle != 0)
  {
    di::CloseCauldronProject(m_projectHandle);
    m_projectHandle = 0;

    m_ui.widget->getViewer()->setSceneGraph(0);
    m_ui.treeWidget->clear();
  }
}

void MainWindow::updateUI()
{
  m_ui.treeWidget->clear();

  QTreeWidgetItem* formationsItem = new QTreeWidgetItem;
  formationsItem->setText(0, "Formations");

  QTreeWidgetItem* surfacesItem = new QTreeWidgetItem;
  surfacesItem->setText(0, "Surfaces");

  QTreeWidgetItem* propertiesItem = new QTreeWidgetItem;
  propertiesItem->setText(0, "Properties");

  int flags = di::FORMATION;
  int type = di::VOLUME;

  std::shared_ptr<di::PropertyList> properties(m_projectHandle->getProperties(true, flags));

  for(size_t i=0; i < properties->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(propertiesItem, TreeWidgetItem_PropertyType);
    item->setText(0, (*properties)[i]->getName().c_str());
  }

  std::shared_ptr<di::FormationList> formations(m_projectHandle->getFormations());
  for(size_t i=0; i < formations->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(formationsItem, TreeWidgetItem_FormationType);
    item->setText(0, (*formations)[i]->getName().c_str());
  }

  std::shared_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces());
  for(size_t i=0; i < surfaces->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(surfacesItem, TreeWidgetItem_SurfaceType);
    item->setText(0, (*surfaces)[i]->getName().c_str());
  }

  m_ui.treeWidget->addTopLevelItem(formationsItem);
  m_ui.treeWidget->addTopLevelItem(surfacesItem);
  m_ui.treeWidget->addTopLevelItem(propertiesItem);
}

void MainWindow::connectSignals()
{
  connect(m_ui.action_Open, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
  connect(m_ui.snapshotSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
  connect(m_ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
}

void MainWindow::onActionOpenTriggered()
{
  QString caption = "Open file";
  QString dir;
  QString filter = "Cauldron project (*.project3d)";
  QString filename = QFileDialog::getOpenFileName(this, "Open file", dir, filter);

  if(!filename.isNull())
  {
    loadProject(filename);
  }
}

void MainWindow::onSliderValueChanged(int value)
{
  m_sceneGraph->setCurrentSnapshot(value);
}

void MainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  if(item->type() == TreeWidgetItem_PropertyType)
  {
    const di::Property* prop = m_projectHandle->findProperty(item->text(0).toStdString());

    if(prop != 0)
    {
      m_sceneGraph->setProperty(prop);
    }
  }
}

MainWindow::MainWindow()
  : m_fpsLabel(0)
  , m_projectHandle(0)
{
	m_ui.setupUi(this);

	// Remove all the ugly buttons and scroll wheels that 
	// you always get for free with these OIV viewers
	m_ui.widget->setDecoration(false);
  m_ui.widget->getViewer()->setBackgroundColor(SbColor(.1f, .1f, .2f));
  
  SoQtViewer* viewer = dynamic_cast<SoQtViewer*>(m_ui.widget->getViewer());
  if(viewer != 0)
  {
    m_fpsLabel = new QLabel;
    m_fpsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    statusBar()->addPermanentWidget(m_fpsLabel);
    viewer->setFramesPerSecondCallback(fpsCallback, this);
  }

  connectSignals();
}