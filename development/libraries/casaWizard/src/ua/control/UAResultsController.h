//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the uncertainty analysis tab in the thermal wizard
#pragma once

#include "UAResultsTargetTableController.h"

#include <QObject>

namespace casaWizard
{

class ScriptRunController;

namespace ua
{

class UAResultsTab;
class UAScenario;

class UAResultsController : public QObject
{
   Q_OBJECT

public:
   UAResultsController(UAResultsTab* uaResultsTab,
                  UAScenario& casaScenario,
                  ScriptRunController& scriptRunController,
                  QObject* parent);

public slots:
   void slotProjectOpened();

private slots:
   void slotPushButtonMCMCrunCasaClicked();
   void slotPredictionTargetClicked(int row);
   void slotPushButtonExportMcmcOutputClicked();
   void slotPushButtonExportOptimalCasesClicked();
   void slotPushButtonRunOptimalCasesClicked();
   void slotSliderHistogramsChanged(int indexTime);
   void slotCheckBoxHistoryPlotModeChanged(const int checkState);
   void slotPushButtonRecalculateResponseSurfacesClicked();

   void slotUpdateTabGUI(int tabID);

private:
   void refreshGUI();

   void loadObservablesOptimalRun();
   void showOptimalCaseQuality();

   QVector<double> getOptimalSettings(QStringList& parameterNames) const;

   UAResultsTab* m_uaResultsTab;
   UAScenario& m_casaScenario;
   ScriptRunController& m_scriptRunController;
   UAResultsTargetTableController m_targetTableController;
};

} // namespace ua

} // namespace casaWizard
