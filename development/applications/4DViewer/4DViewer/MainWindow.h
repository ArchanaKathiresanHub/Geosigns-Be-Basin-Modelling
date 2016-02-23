//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "ui_MainWindow.h"

#include <Project.h>
#include <SceneGraphManager.h>

#include <QtGui/QMainWindow>

class SoRenderArea;
class SceneGraph;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

  Ui::MainWindow m_ui;

  bool m_oivLicenseOK;

  QLabel* m_snapshotCountLabel;
  QLabel* m_dimensionsLabel;
  QLabel* m_timeLabel;
  QLabel* m_fpsLabel;

  std::shared_ptr<Project> m_project;
  std::shared_ptr<SceneGraphManager> m_sceneGraphManager;
  
  Project::ProjectInfo m_projectInfo;

  SceneGraphManager::ColorScaleParams m_colorScaleParams;

  //static void fpsCallback(float fps, void* userData, SoQtViewer* viewer);

  void initOIV();

  void onFps(float fps);

  void loadProject(const QString& filename);

  void closeProject();

  void enableUI(bool enabled);

  void updateUI();

  void connectSignals();

  int getFaultIndex(const std::string& collectionName, const std::string& faultName) const;

private slots:

  void onActionOpenTriggered();

  void onActionRenderAllSnapshotsTriggered();

  void onActionRenderAllSlicesTriggered();

  void onActionSwitchPropertiesTriggered();

  void onActionViewAllTriggered();

  void onActionViewPresetTriggered();

  void onSliderValueChanged(int value);

  void onSliceIValueChanged(int value);

  void onSliceJValueChanged(int value);

  void onVerticalScaleSliderValueChanged(int value);

  void onSliceToggled(bool value);

  void onRenderStyleChanged();

  void onCoordinateGridToggled(bool value);

  void onProjectionIndexChanged(int index);

  void onColorScaleMappingChanged(int index);

  void onColorScaleRangeChanged(int index);

  void onColorScaleValueChanged();

  void onTransparencyChanged(int value);

  void onTrapsToggled(bool value);

  void onTrapOutlinesToggled(bool value);

  void onDrainageAreaOutlineToggled(bool value);

  void onDrainageAreaTypeChanged(bool value);

  void onFluidContactsToggled(bool value);

  void onFlowLinesStepChanged(int value);

  void onFlowLinesThresholdChanged(int value);

  void onItemDoubleClicked(QTreeWidgetItem* item, int column);

  void onShowGLInfo();

  void onTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

public:

	MainWindow();

    void viewAll();
};

#endif