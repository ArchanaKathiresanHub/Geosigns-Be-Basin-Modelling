#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "MainWindow.ui.h"

#include "Interface/Interface.h"

#include <QtGui/QMainWindow>

class SceneGraph;
class QLabel;

class MainWindow : public QMainWindow
{
	Q_OBJECT;

	Ui::MainWindow m_ui;

  QLabel* m_dimensionsLabel;
  QLabel* m_timeLabel;
  QLabel* m_fpsLabel;

  DataAccess::Interface::ProjectHandle* m_projectHandle;
  
  SceneGraph* m_sceneGraph;

  static void fpsCallback(float fps, void* userData, SoQtViewer* viewer);

  void onFps(float fps);

  void loadProject(const QString& filename);

  void closeProject();

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

  void onRenderModeToggled(bool value);

  void onMeshModeToggled(bool value);

  void onRenderStyleChanged();

  void onItemDoubleClicked(QTreeWidgetItem* item, int column);

  void onShowGLInfo();

public:

	MainWindow();
};

#endif
