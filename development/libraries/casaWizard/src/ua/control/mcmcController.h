// Controller for the uncertainty analysis tab in the thermal wizard
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ScriptRunController;

namespace ua
{

class MCMCTab;
class UAScenario;

class MCMCController : public QObject
{
  Q_OBJECT

public:
  MCMCController(MCMCTab* mcmcTab,
               UAScenario& casaScenario,
               ScriptRunController& scriptRunController,
               QObject* parent);

private slots:
  void slotPushButtonMCMCrunCasaClicked();
  void slotTablePredictionTargetClicked(int row, int column);
  void slotPushButtonExportOptimalCasesClicked();
  void slotPushButtonRunOptimalCasesClicked();
  void slotSliderHistogramsChanged(int indexTime);
  void slotCheckBoxHistoryPlotModeChanged(const int checkState);
  void slotPushButtonAddOptimalDesignPointClicked();

  void slotRefresh();
  void slotUpdateTabGUI(int tabID);

private:
  MCMCTab* mcmcTab_;
  UAScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
};

} // namespace ua

} // namespace casaWizard
