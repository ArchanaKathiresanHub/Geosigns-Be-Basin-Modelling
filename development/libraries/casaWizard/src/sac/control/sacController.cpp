//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "sacController.h"

#include "control/calibrationTargetController.h"
#include "control/casaScriptWriter.h"
#include "control/dataExtractionController.h"
#include "control/functions/copyCaseFolder.h"
#include "control/lithofractionController.h"
#include "control/objectiveFunctionController.h"
#include "control/scriptRunController.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/logger.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/output/workspaceGenerator.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/depthCalibrationScript.h"
#include "model/script/sacScript.h"
#include "view/sacTab.h"
#include "view/sacTabIDs.h"
#include "view/workspaceDialog.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>



namespace casaWizard
{

namespace sac
{

SACcontroller::SACcontroller(SACtab* sacTab,
                             SACScenario& casaScenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent) :
  QObject(parent),
  sacTab_{sacTab},
  casaScenario_{casaScenario},
  scriptRunController_{scriptRunController},
  calibrationTargetController_{new CalibrationTargetController(sacTab->calibrationTargetTable(), casaScenario_, this)},
  dataExtractionController_{new DataExtractionController(casaScenario_, scriptRunController_, this)},
  lithofractionController_{new LithofractionController(sacTab->lithofractionTable() , casaScenario_, this)},
  objectiveFunctionController_{new ObjectiveFunctionController(sacTab->objectiveFunctionTable(), casaScenario_.calibrationTargetManager(), this)}
{
  sacTab_->lineEditProject3D()->setText("");
  sacTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

  connect(sacTab_->pushButtonSACrunCASA(),  SIGNAL(clicked()),                   this, SLOT(slotPushButtonSACrunCasaClicked()));
  connect(sacTab_->pushSelectProject3D(),   SIGNAL(clicked()),                   this, SLOT(slotPushSelectProject3dClicked()));
  connect(sacTab_->pushSelectCalibration(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectCalibrationClicked()));
  connect(sacTab_->lineEditProject3D(),     SIGNAL(textChanged(QString)),        this, SLOT(slotLineEditProject3dTextChanged(QString)));
  connect(sacTab_->lineEditCalibration(),   SIGNAL(textChanged(QString)),        this, SLOT(slotLineEditCalibrationTextChanged(QString)));
  connect(sacTab_->comboBoxCluster(),       SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxClusterCurrentTextChanged(QString)));
  connect(sacTab_->comboBoxApplication(),   SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxApplicationChanged(QString)));
}

void SACcontroller::refreshGUI()
{
  QSignalBlocker blocker(sacTab_->lineEditProject3D());
  QSignalBlocker blocker2(sacTab_->lineEditCalibration());
  sacTab_->lineEditProject3D()->setText(casaScenario_.project3dPath());
  sacTab_->lineEditCalibration()->setText("");
  sacTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());
  sacTab_->comboBoxApplication()->setCurrentText(casaScenario_.applicationName());
  lithofractionController_->updateLithofractionTable();

  emit signalRefreshChildWidgets();
}

void SACcontroller::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::SAC))
  {
    return;
  }

  refreshGUI();
}

void SACcontroller::slotExtractAfterOpen()
{
  dataExtractionController_->readResults();
}

void SACcontroller::slotPushButtonSACrunCasaClicked()
{
  wellTrajectoryWriter::writeTrajectories(casaScenario_);

  const QString workingDir = casaScenario_.workingDirectory();
  const QString calibrationDir{casaScenario_.calibrationDirectory()};

  QDir calDir(calibrationDir);
  if (calDir.exists())
  {
    QMessageBox removeFolder(QMessageBox::Icon::Question,
                             "Destination folder exists",
                             "Do you want to empty the destination folder? This removes previous data, but prevents possible conflicts.",
                             QMessageBox::Yes | QMessageBox::No );
    if (removeFolder.exec() == QMessageBox::Yes)
    {
      QDir(calibrationDir).removeRecursively();
      calDir.mkpath(".");
    }
  }
  else
  {
    calDir.mkpath(".");
  }

  const bool filesCopied = functions::copyCaseFolder(workingDir, calibrationDir);

  Logger::log() << (filesCopied ? "Finished copying case to " + calibrationDir :
                                  "Failed copying case, no files were copied") << Logger::endl();

  scenarioBackup::backup(casaScenario_);
  SACScript sac{casaScenario_, calibrationDir};
  if (!casaScriptWriter::writeCasaScript(sac))
  {
    return;
  }

  if (scriptRunController_.runScript(sac))
  {
    dataExtractionController_->readResults();

    if (QFile::copy(casaScenario_.calibrationDirectory() + "/" + casaScenario_.stateFileNameSAC() ,
                    casaScenario_.calibrationDirectory() + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameSAC()))
    {
      QFile file (casaScenario_.calibrationDirectory() + "/" + casaScenario_.stateFileNameSAC());
      file.remove();
    }
    else
    {
      Logger::log() << "- An error occurred while moving the state file to the last iteration folder." << Logger::endl();
    }
    scenarioBackup::backup(casaScenario_);
  }
}

void SACcontroller::slotPushSelectProject3dClicked()
{
  QString fileName = QFileDialog::getOpenFileName(sacTab_,
                                                  "Select project file",
                                                  casaScenario_.project3dPath(),
                                                  "Project files (*.project3d *.project1d)");
  if (fileName == "")
  {
    return;
  }

  QDir fileNamePath = QFileInfo(fileName).absoluteDir();
  QString originalWorkspaceLocation = fileNamePath.absolutePath();

  WorkspaceDialog popupWorkspace{originalWorkspaceLocation, casaWizard::workspaceGenerator::getSuggestedWorkspace(fileName) };

  if (popupWorkspace.exec() != QDialog::Accepted)
  {
    return;
  }

  casaWizard::workspaceGenerator::createWorkspace(originalWorkspaceLocation, popupWorkspace.optionSelected());

  casaScenario_.setWorkingDirectory(popupWorkspace.optionSelected());
  casaScenario_.setProject3dFilePath(fileName);

  const QString workingDir = casaScenario_.workingDirectory();
  const QString projectName = casaScenario_.project3dFilename();

  sacTab_->lineEditProject3D()->setText(workingDir + "/" + projectName);

  lithofractionController_->loadLayersFromProject();
}

void SACcontroller::slotPushSelectCalibrationClicked()
{
  QString fileName = QFileDialog::getOpenFileName(sacTab_,
                                                  "Select calibration targets",
                                                  "",
                                                  "Spreadsheet (*.xlsx)");
  sacTab_->lineEditCalibration()->setText(fileName);
}

void SACcontroller::slotLineEditProject3dTextChanged(QString project3dPath)
{
  casaScenario_.setProject3dFilePath(project3dPath);
}

void SACcontroller::slotLineEditCalibrationTextChanged(QString calibrationTargetsFilename)
{
  calibrationTargetCreator::createFromExcel(casaScenario_, calibrationTargetsFilename);

  emit signalRefreshChildWidgets();

  WellTrajectoryManager& wtManager = casaScenario_.wellTrajectoryManager();

  wtManager.updateWellTrajectories(casaScenario_.calibrationTargetManager());

  casaScenario_.updateRelevantProperties();
}

void SACcontroller::slotComboBoxClusterCurrentTextChanged(QString clusterName)
{
  casaScenario_.setClusterName(clusterName);
}

void SACcontroller::slotComboBoxApplicationChanged(QString application)
{
  if (application == "Iteratively Coupled")
  {
    casaScenario_.setApplicationName("fastcauldron \"-itcoupled\"");
  }
  else if (application == "Hydrostatic")
  {
    casaScenario_.setApplicationName("fastcauldron \"-temperature\"");
  }
  else
  {
    casaScenario_.setApplicationName("");
  }
}

} // namespace sac

} // namespace casaWizard
