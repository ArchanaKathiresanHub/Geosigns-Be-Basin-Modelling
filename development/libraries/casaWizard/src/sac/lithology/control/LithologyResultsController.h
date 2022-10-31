//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/SacResultsController.h"
#include "model/LithologyScenario.h"
#include "model/wellTrajectoryManager.h"
#include "view/LithologyResultsTab.h"

class QString;

namespace casaWizard
{

class CalibrationTarget;
class CalibrationTargetManager;
class ScriptRunController;

namespace sac
{

namespace lithology
{

class LithologyResultsController : public SacResultsController
{
   Q_OBJECT

public:
   explicit LithologyResultsController(LithologyResultsTab* resultsTab,
                                       LithologyScenario& scenario,
                                       ScriptRunController& scriptRunController,
                                       QObject* parent);
private slots:
   void slotUpdateTabGUI(int tabID) final;
   void slotTogglePlotType(const int currentIndex) final;
   void slotWellPrepOrSAC(int buttonId);

private:
   LithologyScenario& scenario() final;
   LithologyScenario& scenario() const final;

   LithologyResultsTab* resultsTab() final;
   const LithologyResultsTab* resultsTab() const final;

   void updateOptimizedTable() final;

   LithologyResultsTab* m_resultsTab;
   LithologyScenario& m_scenario;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
