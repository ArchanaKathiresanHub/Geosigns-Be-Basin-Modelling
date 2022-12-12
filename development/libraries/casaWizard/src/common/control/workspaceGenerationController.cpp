//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "workspaceGenerationController.h"

#include "model/functions/folderOperations.h"
#include "model/logger.h"
#include "model/output/workspaceGenerator.h"
#include "model/casaScenario.h"
#include "view/workspaceDialog.h"

#include <QDir>
#include <QFileDialog>

namespace casaWizard
{

namespace workspaceGenerationController
{

bool generateWorkSpace(QString directory, CasaScenario& scenario)
{
  QString fileDialogLocation = directory == "" ? scenario.defaultFileDialogLocation() : directory;

  const QDir casaCaseDir(QFileDialog::getExistingDirectory(nullptr, "Select your workspace directory", fileDialogLocation,
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

  WorkspaceDialog popupWorkspace{casaCaseDir.absolutePath(), workspaceGenerator::getSuggestedWorkspace(casaCaseDir.absolutePath())};
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
} // namespace casaWizard
