#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "MainWindow.ui.h"

#include <Visualization/SceneGraphManager.h>

#include <Interface/Interface.h>

#include <QtGui/QMainWindow>

class SceneGraph;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

  Ui::MainWindow m_ui;

  bool m_oivLicenseOK;

  QLabel* m_dimensionsLabel;
  QLabel* m_timeLabel;
  QLabel* m_fpsLabel;

  std::unique_ptr<DataAccess::Interface::ObjectFactory> m_factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> m_projectHandle;
  
  SceneGraph* m_sceneGraph;
  SceneGraphManager m_sceneGraphManager;

  static void fpsCallback(float fps, void* userData, SoQtViewer* viewer);

  void initOIV();

  void onFps(float fps);

  void loadProject(const QString& filename);

  void closeProject();

  void enableUI(bool enabled);

  void updateUI();

  void connectSignals();

private slots:

  void onActionOpenTriggered();

  void onActionRenderAllSnapshotsTriggered();

  void onActionRenderAllSlicesTriggered();

  void onActionSwitchPropertiesTriggered();

  void onSliderValueChanged(int value);

  void onSliceIValueChanged(int value);

  void onSliceJValueChanged(int value);

  void onVerticalScaleSliderValueChanged(int value);

  void onROISliderValueChanged(int value);

  void onROIToggled(bool value);

  void onSliceToggled(bool value);

  void onRenderStyleChanged();

  void onItemDoubleClicked(QTreeWidgetItem* item, int column);

  void onShowGLInfo();

  void onTreeWidgetItemChanged(QTreeWidgetItem* item, int column);

public:

	MainWindow();
};

#endif
