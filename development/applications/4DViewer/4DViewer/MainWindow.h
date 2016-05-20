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
#include <SceneExaminer.h>

#include <Inventor/misc/SoRef.h>

#include <QtWidgets/QMainWindow>

class SceneGraph;
class SeismicScene;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

  Ui::MainWindow m_ui;

  bool m_oivLicenseOK;

  QLabel* m_modeLabel;
  QLabel* m_snapshotCountLabel;
  QLabel* m_dimensionsLabel;
  QLabel* m_timeLabel;
  QLabel* m_fpsLabel;

  std::shared_ptr<Project> m_project;
  std::shared_ptr<SceneGraphManager> m_sceneGraphManager;
  std::shared_ptr<SeismicScene> m_seismicScene;

  SoRef<SceneExaminer> m_examiner;

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

  void onModeChanged(SceneExaminer::InteractionMode mode);

  void onFenceAdded(int fenceId);

private slots:

  void onActionOpenTriggered();

  void onActionImportSeismicTriggered();

  void onActionExportSeismicTriggered();

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

  void onCellFilterToggled(bool value);

  void onCellFilterRangeChanged();

  void onItemDoubleClicked(QTreeWidgetItem* item, int column);

  void onShowGLInfo();

  void onShowIvTune();

  void onTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

  void onFenceListItemClicked(QListWidgetItem* item);

  void onSeismicSliceToggled(bool value);

  void onSeismicSliceValueChanged(int value);

  void onSeismicColorScaleValueChanged();

  void onLoadSeismicColorMapClicked();

  void onInterpolatedSurfaceToggled(bool value);

  void onInterpolatedSurfacePositionChanged(int value);

public:

	MainWindow();

    void viewAll();
};

#endif
