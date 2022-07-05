// Controller for the uncertainty analysis tab in the thermal wizard
#pragma once

#include <QObject>

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
   void slotPushButtonExportMcmcOutputClicked();
   void slotPushButtonExportOptimalCasesClicked();
   void slotPushButtonRunOptimalCasesClicked();
   void slotSliderHistogramsChanged(int indexTime);
   void slotCheckBoxHistoryPlotModeChanged(const int checkState);
   void slotPushButtonAddOptimalDesignPointClicked();

   void slotUpdateTabGUI(int tabID);

private:
   void refreshGUI();

   bool doesOptimalCaseExist() const;

   MCMCTab* m_mcmcTab;
   UAScenario& m_casaScenario;
   ScriptRunController& m_scriptRunController;
};

} // namespace ua

} // namespace casaWizard
