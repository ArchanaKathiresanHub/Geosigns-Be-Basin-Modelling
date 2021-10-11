//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellPrepSACcontroller.h"

#include "control/scriptRunController.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "view/calibrationTargetTable.h"
#include "view/sacTabIDs.h"
#include "view/wellPrepTab.h"

#include <QPushButton>

namespace casaWizard
{

namespace sac
{

WellPrepSACcontroller::WellPrepSACcontroller(WellPrepTab* wellPrepTab,
                                             SACScenario& scenario,
                                             ScriptRunController& scriptRunController,
                                             QObject* parent):
  WellPrepController(wellPrepTab, scenario, scriptRunController, parent),
  scenario_{scenario}
{
  connect(wellPrepTab->buttonToSAC(), SIGNAL(clicked()), this, SLOT(slotToSAC()));
}

bool WellPrepSACcontroller::checkTabID(int tabID) const
{
  return (tabID == static_cast<int>(TabID::WellPrep));
}

void WellPrepSACcontroller::slotToSAC()
{
  Logger::log() << "Importing selected wells to the SAC workflow..." << Logger::endl();
  if (scenario_.project3dPath().isEmpty())
  {
    return;
  }

  scenario_.calibrationTargetManager().appendFrom(scenario_.calibrationTargetManagerWellPrep());
  scenario_.calibrationTargetManager().disableInvalidWells(scenario_.project3dPath().toStdString(), scenario_.projectReader().getDepthGridName(0).toStdString());

  scenario_.updateObjectiveFunctionFromTargets();

  if (scenario_.objectiveFunctionManager().indexOfCauldronName("Velocity") != -1)
  {
    QMessageBox velocityDisabled(QMessageBox::Icon::Information,
                          "Velocity calibration data disabled",
                          "It is not possible to optimize using velocity calibration data. If you want to use the velocity data, first convert to SonicSlowness (DT)",
                          QMessageBox::Ok);
    velocityDisabled.exec();
  }

  WellTrajectoryManager& wtManager = scenario_.wellTrajectoryManager();
  wtManager.updateWellTrajectories(scenario_.calibrationTargetManager());

  scenarioBackup::backup(scenario_);  
  Logger::log() << "Done!" << Logger::endl();

  emit switchToTab(static_cast<int>(TabID::SAC));
}

} // namespace sac

} // namespace casaWizard
