//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MainWindow.h"
#include "GLInfoDialog.h"
  
#include <Interface/ProjectHandle.h>
#include <Interface/ObjectFactory.h>
#include "Interface/Property.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/Grid.h"
#include "Interface/FaultCollection.h"
#include "Interface/Faulting.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QLabel>
#include <QtCore/QTime>

#include <MeshVizXLM/mapping/MoMeshviz.h>
#include <MeshViz/PoMeshViz.h>

#include <Visualization/CameraUtil.h>

namespace di = DataAccess::Interface;

void exportData(const di::ProjectHandle* handle);

namespace
{
  const int TreeWidgetItem_FormationType = QTreeWidgetItem::UserType + 1;
  const int TreeWidgetItem_SurfaceType   = QTreeWidgetItem::UserType + 2;
  const int TreeWidgetItem_PropertyType  = QTreeWidgetItem::UserType + 3;
  const int TreeWidgetItem_ReservoirType = QTreeWidgetItem::UserType + 4;
  const int TreeWidgetItem_FaultType     = QTreeWidgetItem::UserType + 5;
  const int TreeWidgetItem_FaultCollectionType = QTreeWidgetItem::UserType + 6;
  const int TreeWidgetItem_FormationGroupType  = QTreeWidgetItem::UserType + 7;
  const int TreeWidgetItem_SurfaceGroupType    = QTreeWidgetItem::UserType + 8;
}

void MainWindow::fpsCallback(float fps, void* userData, SoQtViewer* viewer)
{
  MainWindow* mainWnd = reinterpret_cast<MainWindow*>(userData);
  mainWnd->onFps(fps);
}

void MainWindow::initOIV()
{
  char* features[] =
  {
    "OpenInventor",
    "MeshVizXLM"
  };

  float oivVersion = SoDB::getLicensingVersionNumber();

  m_oivLicenseOK = true;
  for (int i = 0; i < 2; ++i)
  if (SoDB::LicenseCheck(features[i], oivVersion, nullptr, false, nullptr) < 0)
    m_oivLicenseOK = false;

  if (m_oivLicenseOK)
  {
    MoMeshViz::init();
    PoMeshViz::init();
  }
}

void MainWindow::onFps(float fps)
{
  m_fpsLabel->setText(QString("%1 fps").arg(fps));
}

void MainWindow::loadProject(const QString& filename)
{
  closeProject();

  {
    //std::string file = filename.toStdString();
    QByteArray barray = filename.toLatin1();
    const char* str = barray.data();
    std::cout << "Loading project..." << std::endl;
    m_projectHandle.reset(di::OpenCauldronProject(str, "r", m_factory.get()));
    std::cout << "...done" << std::endl;
  }

  setWindowFilePath(filename);

  if (m_oivLicenseOK)
  {
    m_sceneGraphManager.setup(m_projectHandle.get());
    m_sceneGraphManager.setProjection(SceneGraphManager::PerspectiveProjection);

    m_ui.renderWidget->getViewer()->setSceneGraph(m_sceneGraphManager.getRoot());

    m_ui.snapshotSlider->setMinimum(0);
    m_ui.snapshotSlider->setMaximum((int)m_sceneGraphManager.getSnapshotCount() - 1);
    m_ui.snapshotSlider->setValue(m_ui.snapshotSlider->maximum());

    m_ui.renderWidget->getViewer()->getGuiAlgoViewers()->viewAll();
  }

  updateUI();
}

void MainWindow::closeProject()
{
  if(m_projectHandle != 0)
  {
    //di::CloseCauldronProject(m_projectHandle.get());
    //m_projectHandle = nullptr;

    m_sceneGraph = nullptr;

    m_ui.renderWidget->getViewer()->setSceneGraph(nullptr);
    m_ui.treeWidget->clear();
    m_ui.treeWidgetProperties->clear();
  }
}

void MainWindow::enableUI(bool enabled)
{
  m_ui.groupBox->setEnabled(enabled);
  m_ui.groupBox_3->setEnabled(enabled);

  m_ui.sliderVerticalScale->setEnabled(enabled);
  m_ui.snapshotSlider->setEnabled(enabled);
}

void MainWindow::updateUI()
{
  m_ui.treeWidget->clear();

  QTreeWidgetItem* formationsItem = new QTreeWidgetItem(TreeWidgetItem_FormationGroupType);
  formationsItem->setText(0, "Formations");
  formationsItem->setCheckState(0, Qt::Checked);
  QFont font = formationsItem->font(0);
  font.setBold(true);
  formationsItem->setFont(0, font);

  QTreeWidgetItem* surfacesItem = new QTreeWidgetItem(TreeWidgetItem_SurfaceGroupType);
  surfacesItem->setText(0, "Surfaces");
  surfacesItem->setCheckState(0, Qt::Unchecked);
  surfacesItem->setFont(0, font);

  QTreeWidgetItem* reservoirsItem = new QTreeWidgetItem;
  reservoirsItem->setText(0, "Reservoirs");
  reservoirsItem->setFont(0, font);

  m_ui.treeWidgetProperties->clear();
  QTreeWidgetItem* header = m_ui.treeWidgetProperties->headerItem();
  header->setText(0, "Name");
  header->setText(1, "Unit");
  header->setText(2, "Type");
  header->setText(3, "Attribute");

  QTreeWidgetItem* propertiesItem = new QTreeWidgetItem;
  propertiesItem->setText(0, "Properties");

  // Add properties to parent node
  std::unique_ptr<di::PropertyList> properties(m_projectHandle->getProperties(true));
  if (properties && !properties->empty())
  {
    for (size_t i = 0; i < properties->size(); ++i)
    {
      const di::Property* prop = (*properties)[i];

      const int allFlags = di::FORMATION | di::SURFACE | di::RESERVOIR | di::FORMATIONSURFACE;
      const int allTypes = di::MAP | di::VOLUME;
      if (!prop->hasPropertyValues(allFlags, 0, 0, 0, 0, allTypes))
        continue;

      QTreeWidgetItem* item = new QTreeWidgetItem(propertiesItem, TreeWidgetItem_PropertyType);
      item->setText(0, prop->getName().c_str());
      item->setText(1, prop->getUnit().c_str());

      di::PropertyType type = prop->getType();
      QString typeStr = "";
      switch (type)
      {
      case di::FORMATIONPROPERTY: typeStr = "Formation"; break;
      case di::RESERVOIRPROPERTY: typeStr = "Reservoir"; break;
      case di::TRAPPROPERTY: typeStr = "Trap"; break;
      }
      item->setText(2, typeStr);

      DataModel::PropertyAttribute attr = prop->getPropertyAttribute();
      QString attrStr = "";
      switch (attr)
      {
      case DataModel::CONTINUOUS_3D_PROPERTY:     attrStr = "Continuous 3D"; break;
      case DataModel::DISCONTINUOUS_3D_PROPERTY:  attrStr = "Discontinuous 3D"; break;
      case DataModel::SURFACE_2D_PROPERTY:        attrStr = "Surface 2D"; break;
      case DataModel::FORMATION_2D_PROPERTY:      attrStr = "Formation 2D"; break;
      case DataModel::TRAP_PROPERTY:              attrStr = "Trap"; break;
      case DataModel::UNKNOWN_PROPERTY_ATTRIBUTE: attrStr = "Unknown"; break;
      }
      item->setText(3, attrStr);
    }
  }

  m_ui.treeWidgetProperties->addTopLevelItem(propertiesItem);
  propertiesItem->setExpanded(true);

  // Add formations to parent node
  std::unique_ptr<di::FormationList> formations(m_projectHandle->getFormations(nullptr, false));
  if (formations && !formations->empty())
  {
    for (size_t i = 0; i < formations->size(); ++i)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(formationsItem, TreeWidgetItem_FormationType);
      item->setText(0, (*formations)[i]->getName().c_str());
      item->setCheckState(0, Qt::Checked);

      // Add reservoirs to parent formation
      std::unique_ptr<di::ReservoirList> reservoirs((*formations)[i]->getReservoirs());
      if (reservoirs && !reservoirs->empty())
      {
        QTreeWidgetItem* reservoirRoot = new QTreeWidgetItem(item);
        reservoirRoot->setText(0, "Reservoirs");

        for (size_t j = 0; j < reservoirs->size(); ++j)
        {
          QTreeWidgetItem* resItem = new QTreeWidgetItem(reservoirRoot, TreeWidgetItem_ReservoirType);
          resItem->setText(0, (*reservoirs)[j]->getName().c_str());
        }
      }

      // Add faults to parent formation
      std::unique_ptr<di::FaultCollectionList> faultCollections((*formations)[i]->getFaultCollections());
      if (faultCollections && !faultCollections->empty())
      {
        QTreeWidgetItem* faultRoot = new QTreeWidgetItem(item);
        faultRoot->setText(0, "Faults");

        for (size_t j = 0; j < faultCollections->size(); ++j)
        {
          QTreeWidgetItem* faultCollectionItem = new QTreeWidgetItem(faultRoot, TreeWidgetItem_FaultCollectionType);
          faultCollectionItem->setText(0, (*faultCollections)[j]->getName().c_str());
          faultCollectionItem->setCheckState(0, Qt::Unchecked);

          std::unique_ptr<di::FaultList> faults((*faultCollections)[j]->getFaults());
          for (size_t k = 0; k < faults->size(); ++k)
          {
            QTreeWidgetItem* faultItem = new QTreeWidgetItem(faultCollectionItem, TreeWidgetItem_FaultType);
            faultItem->setText(0, (*faults)[k]->getName().c_str());
            faultItem->setCheckState(0, Qt::Unchecked);
          }
        }
      }
    }
  }

  // Add surfaces to parent node
  std::unique_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces());
  if (surfaces && !surfaces->empty())
  {
    for (size_t i = 0; i < surfaces->size(); ++i)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(surfacesItem, TreeWidgetItem_SurfaceType);
      item->setText(0, (*surfaces)[i]->getName().c_str());
      item->setCheckState(0, Qt::Unchecked);
    }
  }

  // Add reservoirs to parent node
  std::unique_ptr<di::ReservoirList> reservoirs(m_projectHandle->getReservoirs());
  if (reservoirs && !reservoirs->empty())
  {
    for (auto reservoir : *reservoirs)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(reservoirsItem, TreeWidgetItem_ReservoirType);
      item->setText(0, reservoir->getName().c_str());
      item->setCheckState(0, Qt::Unchecked);
    }
  }

  m_ui.treeWidget->addTopLevelItem(formationsItem);
  m_ui.treeWidget->addTopLevelItem(surfacesItem);
  m_ui.treeWidget->addTopLevelItem(reservoirsItem);

  formationsItem->setExpanded(true);
  surfacesItem->setExpanded(true);
  reservoirsItem->setExpanded(true);

  enableUI(true);// m_sceneGraph != nullptr);

  m_snapshotCountLabel->setText(QString("%1 snapshots")
    .arg(m_sceneGraphManager.getSnapshotCount()));
  m_dimensionsLabel->setText(QString("Dimensions: %1x%2 / %3x%4")
    .arg(m_sceneGraphManager.numI())
    .arg(m_sceneGraphManager.numJ())
    .arg(m_sceneGraphManager.numIHiRes())
    .arg(m_sceneGraphManager.numJHiRes()));

  m_ui.sliderSliceI->setMaximum(m_sceneGraphManager.numI() - 2);
  m_ui.sliderSliceJ->setMaximum(m_sceneGraphManager.numJ() - 2);
}

void MainWindow::connectSignals()
{
  connect(m_ui.action_Open, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
  connect(m_ui.action_RenderAllSnapshots, SIGNAL(triggered()), this, SLOT(onActionRenderAllSnapshotsTriggered()));
  connect(m_ui.action_RenderAllSlices, SIGNAL(triggered()), this, SLOT(onActionRenderAllSlicesTriggered()));
  connect(m_ui.action_SwitchProperties, SIGNAL(triggered()), this, SLOT(onActionSwitchPropertiesTriggered()));
  connect(m_ui.action_OpenGLInfo, SIGNAL(triggered()), this, SLOT(onShowGLInfo()));
  connect(m_ui.action_ViewAll, SIGNAL(triggered()), this, SLOT(onActionViewAllTriggered()));

  QAction* actions[] =
  {
    m_ui.action_ViewTop, m_ui.action_ViewLeft, m_ui.action_ViewFront, 
    m_ui.action_ViewBottom, m_ui.action_ViewRight, m_ui.action_ViewBack
  };
  for (int i = 0; i < 6; ++i)
    connect(actions[i], SIGNAL(triggered()), this, SLOT(onActionViewPresetTriggered()));

  connect(m_ui.snapshotSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
  connect(m_ui.treeWidgetProperties, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

  connect(m_ui.checkBoxSliceI, SIGNAL(toggled(bool)), this, SLOT(onSliceToggled(bool)));
  connect(m_ui.checkBoxSliceJ, SIGNAL(toggled(bool)), this, SLOT(onSliceToggled(bool)));
  connect(m_ui.sliderSliceI, SIGNAL(valueChanged(int)), this, SLOT(onSliceIValueChanged(int)));
  connect(m_ui.sliderSliceJ, SIGNAL(valueChanged(int)), this, SLOT(onSliceJValueChanged(int)));
  connect(m_ui.sliderVerticalScale, SIGNAL(valueChanged(int)), this, SLOT(onVerticalScaleSliderValueChanged(int)));
  connect(m_ui.checkBoxDrawFaces, SIGNAL(toggled(bool)), this, SLOT(onRenderStyleChanged()));
  connect(m_ui.checkBoxDrawEdges, SIGNAL(toggled(bool)), this, SLOT(onRenderStyleChanged()));
  connect(m_ui.checkBoxDrawGrid, SIGNAL(toggled(bool)), this, SLOT(onCoordinateGridToggled(bool)));
  connect(m_ui.checkBoxPerspective, SIGNAL(toggled(bool)), this, SLOT(onPerspectiveToggled(bool)));

  connect(m_ui.checkBoxTraps, SIGNAL(toggled(bool)), this, SLOT(onTrapsToggled(bool)));
  connect(m_ui.checkBoxTrapOutline, SIGNAL(toggled(bool)), this, SLOT(onTrapOutlinesToggled(bool)));
  connect(m_ui.checkBoxDrainageOutline, SIGNAL(toggled(bool)), this, SLOT(onDrainageAreaOutlineToggled(bool)));
  connect(m_ui.radioButtonDrainageAreaFluid, SIGNAL(toggled(bool)), this, SLOT(onDrainageAreaTypeChanged(bool)));
  connect(m_ui.radioButtonDrainageAreaGas, SIGNAL(toggled(bool)), this, SLOT(onDrainageAreaTypeChanged(bool)));

  connect(m_ui.radioButtonFlowVizNone, SIGNAL(toggled(bool)), this, SLOT(onFlowVizTypeChanged(bool)));
  connect(m_ui.radioButtonFlowVizLines, SIGNAL(toggled(bool)), this, SLOT(onFlowVizTypeChanged(bool)));
  connect(m_ui.radioButtonFlowVizVectors, SIGNAL(toggled(bool)), this, SLOT(onFlowVizTypeChanged(bool)));

  connect(m_ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onTreeWidgetItemChanged(QTreeWidgetItem*, int)));
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

void MainWindow::onActionViewAllTriggered()
{
  m_ui.renderWidget->getViewer()->getGuiAlgoViewers()->viewAll();
}

void MainWindow::onActionViewPresetTriggered()
{
  static const QAction* actions[] =
  {
    m_ui.action_ViewTop, m_ui.action_ViewLeft, m_ui.action_ViewFront,
    m_ui.action_ViewBottom, m_ui.action_ViewRight, m_ui.action_ViewBack
  };

  static const ViewPreset presets[] =
  {
    ViewPreset_Top, ViewPreset_Left, ViewPreset_Front,
    ViewPreset_Bottom, ViewPreset_Right, ViewPreset_Back
  };

  for (int i = 0; i < 6; ++i)
  {
    if (sender() == actions[i])
    {
      SoCamera* camera = m_sceneGraphManager.getCamera();
      setViewPreset(camera, presets[i]);
      break;
    }
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

void MainWindow::onActionRenderAllSlicesTriggered()
{
  m_ui.renderWidget->getViewer()->setAutoRedraw(false);

  QTime time;
  time.start();

  int maxTimeMs = 0;
  for(int i=0; i <= m_ui.sliderSliceI->maximum(); ++i)
  {
    QTime snapshotTime;
    snapshotTime.start();

    m_ui.sliderSliceI->setValue(i);
    qApp->processEvents();
    m_ui.renderWidget->getViewer()->render();

    int t = snapshotTime.elapsed();
    if(t > maxTimeMs)
      maxTimeMs = t;
  }

  for(int i=0; i <= m_ui.sliderSliceJ->maximum(); ++i)
  {
    QTime snapshotTime;
    snapshotTime.start();

    m_ui.sliderSliceJ->setValue(i);
    qApp->processEvents();
    m_ui.renderWidget->getViewer()->render();

    int t = snapshotTime.elapsed();
    if(t > maxTimeMs)
      maxTimeMs = t;
  }

  m_ui.renderWidget->getViewer()->setAutoRedraw(true);

  int ms = time.elapsed();
  float avgTime = (.001f * ms) / (m_ui.sliderSliceI->maximum() + m_ui.sliderSliceJ->maximum() + 2);
  float maxTime = .001f * maxTimeMs;
  QString msg = QString("Average time = %1 s per slice\nMax time = %2").arg(avgTime).arg(maxTime);
  QMessageBox::information(this, "Result", msg);
}

void MainWindow::onActionSwitchPropertiesTriggered()
{
  m_ui.renderWidget->getViewer()->setAutoRedraw(false);

  const char* propertyNames[] = 
  {
    "BulkDensity", 
    "Depth",
    "LithoStaticPressure",
    "Permeability",
    "Porosity",
    "Pressure",
    "Reflectivity",
    "Temperature",
    "ThCond",
    "Velocity",
    "Ves",
    "Vr"
  };

  QTime time;
  time.start();

  int n = sizeof(propertyNames) / sizeof(const char*);
  for(int i=0; i < n; ++i)
  {
    const di::Property* prop = m_projectHandle->findProperty(propertyNames[i]);
    m_sceneGraphManager.setProperty(propertyNames[i]);
    qApp->processEvents();
    m_ui.renderWidget->getViewer()->render();
  }

  m_ui.renderWidget->getViewer()->setAutoRedraw(true);

  int ms = time.elapsed();
  float avgTime = (.001f * ms) / n;

  QString msg = QString("Average time = %1 s per property").arg(avgTime);
  QMessageBox::information(this, "Result", msg);
}

void MainWindow::onSliderValueChanged(int value)
{
  //m_timeLabel->setText(QString("Time: %1").arg(m_sceneGraph->getSnapshot(value)->getTime()));
  m_sceneGraphManager.setCurrentSnapshot(value);
}

void MainWindow::onSliceIValueChanged(int value)
{
  m_sceneGraphManager.setSlicePosition(0, value);
}

void MainWindow::onSliceJValueChanged(int value)
{
  m_sceneGraphManager.setSlicePosition(1, value);
}

void MainWindow::onVerticalScaleSliderValueChanged(int value)
{
  float scale = powf(10.f, .2f * value);
  m_sceneGraphManager.setVerticalScale(scale);
}

void MainWindow::onSliceToggled(bool value)
{
  if (sender() == m_ui.checkBoxSliceI)
    m_sceneGraphManager.enableSlice(0, value);
  else
    m_sceneGraphManager.enableSlice(1, value);
}

void MainWindow::onRenderStyleChanged()
{
  bool drawFaces = m_ui.checkBoxDrawFaces->isChecked();
  bool drawEdges = m_ui.checkBoxDrawEdges->isChecked();

  m_sceneGraphManager.setRenderStyle(drawFaces, drawEdges);
}

void MainWindow::onCoordinateGridToggled(bool value)
{
  m_sceneGraphManager.showCoordinateGrid(value);
}

void MainWindow::onPerspectiveToggled(bool value)
{
  m_sceneGraphManager.setProjection(value 
    ? SceneGraphManager::PerspectiveProjection 
    : SceneGraphManager::OrthographicProjection);

  static_cast<SoQtViewer*>(m_ui.renderWidget->getViewer())->setCamera(m_sceneGraphManager.getCamera());
}

void MainWindow::onTrapsToggled(bool value)
{
  m_sceneGraphManager.showTraps(value);
}

void MainWindow::onTrapOutlinesToggled(bool value)
{
  m_sceneGraphManager.showTrapOutlines(value);
}

void MainWindow::onDrainageAreaOutlineToggled(bool value)
{
  bool checked = m_ui.checkBoxDrainageOutline->isChecked();
  m_ui.radioButtonDrainageAreaFluid->setEnabled(checked);
  m_ui.radioButtonDrainageAreaGas->setEnabled(checked);

  SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;
  if (checked)
  {
    if (m_ui.radioButtonDrainageAreaFluid->isChecked())
      type = SceneGraphManager::DrainageAreaFluid;
    else
      type = SceneGraphManager::DrainageAreaGas;
  }

  m_sceneGraphManager.showDrainageAreaOutlines(type);
}

void MainWindow::onDrainageAreaTypeChanged(bool value)
{
  if (value)
  {
    SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;
    if (m_ui.radioButtonDrainageAreaFluid->isChecked())
      type = SceneGraphManager::DrainageAreaFluid;
    else
      type = SceneGraphManager::DrainageAreaGas;

    m_sceneGraphManager.showDrainageAreaOutlines(type);
  }
}

void MainWindow::onFlowVizTypeChanged(bool value)
{
  if (value)
  {
    SceneGraphManager::FlowVizType type = SceneGraphManager::FlowVizNone;
    if (m_ui.radioButtonFlowVizLines->isChecked())
      type = SceneGraphManager::FlowVizLines;
    else if (m_ui.radioButtonFlowVizVectors->isChecked())
      type = SceneGraphManager::FlowVizVectors;

    m_sceneGraphManager.showFlowDirection(type);
  }
}

void MainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  if(item->type() == TreeWidgetItem_PropertyType)
  {
    std::string name = item->text(0).toStdString();
    if (name == "HeatFlow")
    {
      std::string suffix[] = { "X", "Y", "Z" };
      const di::Property* props[3];
      props[0] = m_projectHandle->findProperty(name + "X");
      props[1] = m_projectHandle->findProperty(name + "Y");
      props[2] = m_projectHandle->findProperty(name + "Z");

      bool ok = true;
      for (int i = 0; i < 3; ++i)
      {
        props[i] = m_projectHandle->findProperty(name + suffix[i]);
        ok = ok && (props[i] != 0 && props[i]->hasPropertyValues(di::FORMATION, 0, 0, 0, 0, di::VOLUME));
      }

      if (ok)
        ;// m_sceneGraphManager.setProperty(props);
      else
        std::cout << "No vector property values found for " << name << std::endl;

    }
    else
    {
      m_sceneGraphManager.setProperty(name);
    }
  }
}

void MainWindow::onShowGLInfo()
{
  exportData(m_projectHandle.get());

  GLInfoDialog dlg(this);
  dlg.exec();
}

void MainWindow::onTreeWidgetItemChanged(QTreeWidgetItem* item, int column)
{
  std::string name = item->text(0).toStdString();
  std::string parentName = item->parent() != 0 ? item->parent()->text(0).toStdString() : "";

  bool checked = item->checkState(0) == Qt::Checked;

  switch (item->type())
  {
  case TreeWidgetItem_FormationType:
    m_sceneGraphManager.enableFormation(name, checked);
    break;
  case TreeWidgetItem_SurfaceType:
    m_sceneGraphManager.enableSurface(name, checked);
    break;
  case TreeWidgetItem_ReservoirType:
    m_sceneGraphManager.enableReservoir(name, checked);
    break;
  case TreeWidgetItem_FaultType:
    m_sceneGraphManager.enableFault(parentName, name, checked);
    break;
  case TreeWidgetItem_FaultCollectionType:
  case TreeWidgetItem_FormationGroupType:
  case TreeWidgetItem_SurfaceGroupType:
    for (int i = 0; i < item->childCount(); ++i)
      item->child(i)->setCheckState(0, item->checkState(0));
    break;
  }
}

MainWindow::MainWindow()
  : m_oivLicenseOK(false)
  , m_snapshotCountLabel(nullptr)
  , m_dimensionsLabel(nullptr)
  , m_timeLabel(nullptr)
  , m_fpsLabel(nullptr)
  , m_factory(new di::ObjectFactory)
  , m_projectHandle(nullptr)
  , m_sceneGraph(nullptr)
{
  initOIV();

  m_ui.setupUi(this);

  enableUI(false);

  // Remove all the ugly buttons and scroll wheels that 
  // you always get for free with these OIV viewers
  m_ui.renderWidget->setDecoration(false);
  m_ui.renderWidget->getViewer()->setBackgroundColor(SbColor(.2f, .2f, .3f));

  // Disable OIV widget if there's no valid license, to prevent it from drawing and crashing
  if (!m_oivLicenseOK)
    m_ui.renderWidget->setVisible(false);

  SoQtViewer* viewer = dynamic_cast<SoQtViewer*>(m_ui.renderWidget->getViewer());
  if(viewer != 0)
  {
    viewer->setSceneGraph(nullptr); // avoids annoying 'Qt by Nokia' text in 3D view

    int labelFrameStyle = QFrame::Panel | QFrame::Sunken;

    m_snapshotCountLabel = new QLabel;
    m_snapshotCountLabel->setFrameStyle(labelFrameStyle);

    m_fpsLabel = new QLabel;
    m_fpsLabel->setFrameStyle(labelFrameStyle);

    m_dimensionsLabel = new QLabel;
    m_dimensionsLabel->setFrameStyle(labelFrameStyle);

    m_timeLabel = new QLabel;
    m_timeLabel->setFrameStyle(labelFrameStyle);

    statusBar()->addPermanentWidget(m_snapshotCountLabel);
    statusBar()->addPermanentWidget(m_dimensionsLabel);
    statusBar()->addPermanentWidget(m_timeLabel);
    statusBar()->addPermanentWidget(m_fpsLabel);

    viewer->setFramesPerSecondCallback(fpsCallback, this);
  }

  connectSignals();
}