//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellPrepSACcontroller.h"
#include "workspaceGenerationController.h"

#include "control/scriptRunController.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "view/calibrationTargetTable.h"
#include "view/sacTabIDs.h"
#include "view/wellPrepTab.h"

#include <QDir>
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
  if (scenario_.workingDirectory().isEmpty())
  {
    if (!workspaceGenerationController::generateWorkSpace(QDir::currentPath(),scenario_))
    {
      return;
    }
  }

  Logger::log() << "Importing selected wells to the SAC workflow..." << Logger::endl();  

  if (!scenario_.calibrationTargetManager().wells().empty())
  {
    QMessageBox overwriteData(QMessageBox::Icon::Information,
                              "SAC already has wells",
                              "Would you like to overwrite or append the new wells?");
    QPushButton* appendButton = overwriteData.addButton("Append", QMessageBox::RejectRole);
    QPushButton* overwriteButton =overwriteData.addButton("Overwrite", QMessageBox::AcceptRole);
    connect(appendButton, SIGNAL(clicked()), &overwriteData, SLOT(reject()));
    connect(overwriteButton, SIGNAL(clicked()), &overwriteData, SLOT(accept()));

    if (overwriteData.exec() == QDialog::Accepted)
    {
      scenario_.calibrationTargetManager().clear();
    }
  }

  scenario_.wellPrepToSAC();

  if (scenario_.objectiveFunctionManager().indexOfCauldronName("Velocity") != -1)
  {
    QMessageBox velocityDisabled(QMessageBox::Icon::Information,
                          "Velocity calibration data disabled",
                          "It is not possible to optimize using velocity calibration data. If you want to use the velocity data, first convert to SonicSlowness (DT)",
                          QMessageBox::Ok);
    velocityDisabled.exec();
  }

  scenarioBackup::backup(scenario_);  
  Logger::log() << "Done!" << Logger::endl();

  emit switchToTab(static_cast<int>(TabID::Input));
}

} // namespace sac

} // namespace casaWizard
