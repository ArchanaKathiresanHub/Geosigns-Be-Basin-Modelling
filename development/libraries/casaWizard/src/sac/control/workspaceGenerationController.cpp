//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "workspaceGenerationController.h"

#include "control/functions/folderOperations.h"
#include "model/logger.h"
#include "model/output/workspaceGenerator.h"
#include "model/sacScenario.h"
#include "view/workspaceDialog.h"

namespace casaWizard
{

namespace sac
{

namespace workspaceGenerationController
{

bool generateWorkSpace(QString directory, sac::SACScenario& scenario)
{
  WorkspaceDialog popupWorkspace{QDir::currentPath(), workspaceGenerator::getSuggestedWorkspaceCurrentDirectory()};
  if (popupWorkspace.exec() != QDialog::Accepted)
  {
    return false;
  }

  const QString workingDirectory = popupWorkspace.optionSelected();
  if (!functions::overwriteIfDirectoryExists(workingDirectory))
  {
    return false;
  }

  if (!casaWizard::workspaceGenerator::createWorkspace(directory, workingDirectory))
  {
    Logger::log() << "Unable to create workspace, do you have write access to: " << workingDirectory << "?" << Logger::endl();
    return false;
  }

  scenario.setWorkingDirectory(workingDirectory);
  return true;
}

} // namespace workspacegenerationcontroller
} // namspace sac
} // namespace casaWizard
