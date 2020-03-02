#pragma once

#include <QObject>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SACScenario;
class ResultsTab;

class ResultsController : public QObject
{
  Q_OBJECT

public:
  explicit ResultsController(ResultsTab* resultsTab,
                             SACScenario& scenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent);
  void updateTab();

private slots:
  void updateWell(int wellIndex);
  void refreshPlot();
  void saveOptimized();
  void runOptimized();
  void runBaseCase();
  void togglePlotType(const int currentIndex);
  void updateProperty(const QString property);
  void updateWellFromBirdView(const int lineIndex, const int pointIndex);
  void selectedWellFromScatter(const int wellIndex);

  void slotUpdateTabGUI(int tabID);

private:
  void refreshGUI();
  void updateOptimizedTable();
  void updateWellPlot();
  void updateScatterPlot();
  void updateBirdView();
  bool run3dCase(const QString baseDirectory);
  void import3dWellData(const QString baseDirectory, const bool isOptimized);
  QVector<int> selectedWells();

  ResultsTab* resultsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  int activeWell_;
  QString activeProperty_;

};

} // namespace sac

} // namespace casaWizard
