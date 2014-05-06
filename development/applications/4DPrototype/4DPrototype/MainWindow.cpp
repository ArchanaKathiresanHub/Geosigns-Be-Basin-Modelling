#include "MainWindow.h"
#include "GLInfoDialog.h"
#include <Visualization/SceneGraph.h>

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/Grid.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QLabel>
#include <QtCore/QTime>

namespace di = DataAccess::Interface;

namespace
{
  const int TreeWidgetItem_FormationType = QTreeWidgetItem::UserType + 1;
  const int TreeWidgetItem_SurfaceType   = QTreeWidgetItem::UserType + 2;
  const int TreeWidgetItem_PropertyType  = QTreeWidgetItem::UserType + 3;
  const int TreeWidgetItem_ReservoirType = QTreeWidgetItem::UserType + 4;
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

  m_sceneGraph = new SceneGraph;
  m_sceneGraph->setup(m_projectHandle);
  m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Skin;

  m_ui.renderWidget->getViewer()->setSceneGraph(m_sceneGraph);
  m_ui.snapshotSlider->setMinimum(0);
  m_ui.snapshotSlider->setMaximum(m_sceneGraph->snapshotCount() - 1);
  m_ui.snapshotSlider->setValue(0);
  m_ui.radioButtonSkin->setChecked(true);
  
  updateUI();
}

void MainWindow::closeProject()
{
  if(m_projectHandle != 0)
  {
    di::CloseCauldronProject(m_projectHandle);
    m_projectHandle = 0;

    m_ui.renderWidget->getViewer()->setSceneGraph(0);
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

  QTreeWidgetItem* reservoirsItem = new QTreeWidgetItem;
  reservoirsItem->setText(0, "Reservoirs");

  int flags = di::FORMATION;
  int type = di::VOLUME;

  // Add properties to parent node
  std::shared_ptr<di::PropertyList> properties(m_projectHandle->getProperties(true, flags));
  for(size_t i=0; i < properties->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(propertiesItem, TreeWidgetItem_PropertyType);
    item->setText(0, (*properties)[i]->getName().c_str());
  }

  // Add formations to parent node
  std::shared_ptr<di::FormationList> formations(m_projectHandle->getFormations());
  for(size_t i=0; i < formations->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(formationsItem, TreeWidgetItem_FormationType);
    item->setText(0, (*formations)[i]->getName().c_str());

    // Add reservoirs to parent formation
    di::ReservoirList* reservoirs = (*formations)[i]->getReservoirs();
    for(size_t j=0; j < reservoirs->size(); ++j)
    {
      QTreeWidgetItem* resItem = new QTreeWidgetItem(item, TreeWidgetItem_ReservoirType);
      resItem->setText(0, (*reservoirs)[j]->getName().c_str());
    }
  }

  // Add surfaces to parent node
  std::shared_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces());
  for(size_t i=0; i < surfaces->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(surfacesItem, TreeWidgetItem_SurfaceType);
    item->setText(0, (*surfaces)[i]->getName().c_str());
  }

  // Add reservoirs to parent node
  std::shared_ptr<di::ReservoirList> reservoirs(m_projectHandle->getReservoirs());
  for(size_t i=0; i < reservoirs->size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(reservoirsItem, TreeWidgetItem_ReservoirType);
    item->setText(0, (*reservoirs)[i]->getName().c_str());
  }

  m_ui.treeWidget->addTopLevelItem(formationsItem);
  m_ui.treeWidget->addTopLevelItem(surfacesItem);
  m_ui.treeWidget->addTopLevelItem(reservoirsItem);
  m_ui.treeWidget->addTopLevelItem(propertiesItem);

  const di::Grid* loResGrid = m_projectHandle->getLowResolutionOutputGrid();
  const di::Grid* hiResGrid = m_projectHandle->getHighResolutionOutputGrid();

  m_dimensionsLabel->setText(QString("Dimensions: %1x%2 / %3x%4")
    .arg(loResGrid->numI())
    .arg(loResGrid->numJ())
    .arg(hiResGrid->numI())
    .arg(hiResGrid->numJ()));

  const di::Grid* grid = loResGrid;
  m_ui.sliderSliceI->setMaximum(grid->numI() - 1);
  m_ui.sliderSliceJ->setMaximum(grid->numJ() - 1);

  m_ui.sliderMinI->setMaximum(grid->numI() - 1);
  m_ui.sliderMaxI->setMaximum(grid->numI() - 1);
  m_ui.sliderMinJ->setMaximum(grid->numJ() - 1);
  m_ui.sliderMaxJ->setMaximum(grid->numJ() - 1);
  m_ui.sliderMinK->setMaximum(10);
  m_ui.sliderMaxK->setMaximum(10);

  m_ui.sliderMinI->setValue(0);
  m_ui.sliderMinJ->setValue(0);
  m_ui.sliderMinK->setValue(0);
  m_ui.sliderMaxI->setValue(grid->numI() - 1);
  m_ui.sliderMaxJ->setValue(grid->numJ() - 1);
  m_ui.sliderMaxK->setValue(10); //NOOOOOOO no hardcoded values !!!1!1
}

void MainWindow::connectSignals()
{
  connect(m_ui.action_Open, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
  connect(m_ui.action_RenderAllSnapshots, SIGNAL(triggered()), this, SLOT(onActionRenderAllSnapshotsTriggered()));
  connect(m_ui.action_OpenGLInfo, SIGNAL(triggered()), this, SLOT(onShowGLInfo()));

  connect(m_ui.snapshotSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
  connect(m_ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
  connect(m_ui.sliderSliceI, SIGNAL(valueChanged(int)), this, SLOT(onSliceIValueChanged(int)));
  connect(m_ui.sliderSliceJ, SIGNAL(valueChanged(int)), this, SLOT(onSliceJValueChanged(int)));
  connect(m_ui.radioButtonSkin, SIGNAL(toggled(bool)), this, SLOT(onRenderModeToggled(bool)));
  connect(m_ui.radioButtonSlices, SIGNAL(toggled(bool)), this, SLOT(onRenderModeToggled(bool)));
  connect(m_ui.radioButtonCrossSection, SIGNAL(toggled(bool)), this, SLOT(onRenderModeToggled(bool)));
  connect(m_ui.sliderVerticalScale, SIGNAL(valueChanged(int)), this, SLOT(onVerticalScaleSliderValueChanged(int)));
  connect(m_ui.radioButtonFormations, SIGNAL(toggled(bool)), this, SLOT(onMeshModeToggled(bool)));
  connect(m_ui.radioButtonSurfaces, SIGNAL(toggled(bool)), this, SLOT(onMeshModeToggled(bool)));
  connect(m_ui.radioButtonReservoirs, SIGNAL(toggled(bool)), this, SLOT(onMeshModeToggled(bool)));
  connect(m_ui.checkBoxDrawFaces, SIGNAL(toggled(bool)), this, SLOT(onRenderStyleChanged()));
  connect(m_ui.checkBoxDrawEdges, SIGNAL(toggled(bool)), this, SLOT(onRenderStyleChanged()));

  // ROI
  connect(m_ui.checkBoxROI, SIGNAL(toggled(bool)), this, SLOT(onROIToggled(bool)));
  connect(m_ui.sliderMinI, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
  connect(m_ui.sliderMaxI, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
  connect(m_ui.sliderMinJ, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
  connect(m_ui.sliderMaxJ, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
  connect(m_ui.sliderMinK, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
  connect(m_ui.sliderMaxK, SIGNAL(valueChanged(int)), this, SLOT(onROISliderValueChanged(int)));
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

void MainWindow::onActionRenderAllSnapshotsTriggered()
{
  m_ui.renderWidget->getViewer()->setAutoRedraw(false);

  QTime time;
  time.start();

  int maxTimeMs = 0;
  for(int i=0; i <= m_ui.snapshotSlider->maximum(); ++i)
  {
    QTime snapshotTime;
    snapshotTime.start();

    m_ui.snapshotSlider->setValue(i);
    qApp->processEvents();
    m_ui.renderWidget->getViewer()->render();

    int t = snapshotTime.elapsed();
    if(t > maxTimeMs)
      maxTimeMs = t;
  }

  m_ui.renderWidget->getViewer()->setAutoRedraw(true);

  int ms = time.elapsed();
  float avgTime = (.001f * ms) / (m_ui.snapshotSlider->maximum() + 1);
  float maxTime = .001f * maxTimeMs;
  QString msg = QString("Average time = %1 s per snapshot\nMax time = %2").arg(avgTime).arg(maxTime);
  QMessageBox::information(this, "Result", msg);
}

void MainWindow::onSliderValueChanged(int value)
{
  //m_timeLabel->setText(QString("Time: %1").arg(m_sceneGraph->getSnapshot(value)->getTime()));
  m_sceneGraph->setCurrentSnapshot(value);
}

void MainWindow::onSliceIValueChanged(int value)
{
  m_sceneGraph->SliceI = value;
}

void MainWindow::onSliceJValueChanged(int value)
{
  m_sceneGraph->SliceJ = value;
}

void MainWindow::onVerticalScaleSliderValueChanged(int value)
{
  m_sceneGraph->setVerticalScale((float)value);
}

void MainWindow::onROISliderValueChanged(int value)
{
  m_sceneGraph->setROI(
    (size_t)m_ui.sliderMinI->value(),
    (size_t)m_ui.sliderMinJ->value(),
    (size_t)m_ui.sliderMinK->value(),
    (size_t)m_ui.sliderMaxI->value(),
    (size_t)m_ui.sliderMaxJ->value(),
    (size_t)m_ui.sliderMaxK->value());
}

void MainWindow::onROIToggled(bool value)
{
  m_sceneGraph->enableROI(value);
}

void MainWindow::onRenderModeToggled(bool value)
{
  if(!value)
    return;

  if(sender() == m_ui.radioButtonSkin)
  {
    m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Skin;
    m_sceneGraph->showPlaneManip(false);
  }
  else if(sender() == m_ui.radioButtonSlices)
  {
    m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Slices;
    m_sceneGraph->showPlaneManip(false);
  }
  else
  {
    m_sceneGraph->RenderMode = SnapshotNode::RenderMode_CrossSection;
    m_sceneGraph->showPlaneManip(true);
  }
}

void MainWindow::onMeshModeToggled(bool value)
{
  if(!value)
    return;

  if(sender() == m_ui.radioButtonFormations)
    m_sceneGraph->setMeshMode(SceneGraph::MeshMode_All);
  else
    m_sceneGraph->setMeshMode(SceneGraph::MeshMode_Reservoirs);
}

void MainWindow::onRenderStyleChanged()
{
  bool drawFaces = m_ui.checkBoxDrawFaces->isChecked();
  bool drawEdges = m_ui.checkBoxDrawEdges->isChecked();

  m_sceneGraph->setRenderStyle(drawFaces, drawEdges);
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

void MainWindow::onShowGLInfo()
{
  GLInfoDialog dlg(this);
  dlg.exec();
}

MainWindow::MainWindow()
  : m_fpsLabel(0)
  , m_projectHandle(0)
{
	m_ui.setupUi(this);

	// Remove all the ugly buttons and scroll wheels that 
	// you always get for free with these OIV viewers
	m_ui.renderWidget->setDecoration(false);
  m_ui.renderWidget->getViewer()->setBackgroundColor(SbColor(.2f, .2f, .3f));

  SoQtViewer* viewer = dynamic_cast<SoQtViewer*>(m_ui.renderWidget->getViewer());
  if(viewer != 0)
  {
    viewer->setSceneGraph(0); // avoids annoying 'Qt by Nokia' text in 3D view

    m_fpsLabel = new QLabel;
    m_fpsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    m_dimensionsLabel = new QLabel;
    m_dimensionsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    m_timeLabel = new QLabel;
    m_timeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    statusBar()->addPermanentWidget(m_dimensionsLabel);
    statusBar()->addPermanentWidget(m_timeLabel);
    statusBar()->addPermanentWidget(m_fpsLabel);

    viewer->setFramesPerSecondCallback(fpsCallback, this);
  }

  connectSignals();
}