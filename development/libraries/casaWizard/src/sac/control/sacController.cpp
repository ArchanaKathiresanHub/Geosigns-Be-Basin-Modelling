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
#include "control/functions/folderOperations.h"
#include "control/lithofractionController.h"
#include "control/objectiveFunctionControllerSAC.h"
#include "control/run3dCaseController.h"
#include "control/scriptRunController.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/cmbMapReader.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/cmbProjectWriter.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/output/workspaceGenerator.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/depthConversionScript.h"
#include "model/script/sacScript.h"
#include "model/functions/sortedByXWellIndices.h"
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
#include <QStringList>
#include <QTableWidgetItem>
#include <QVector>
#include <QThread>

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
  lithofractionController_{new LithofractionController(sacTab->lithofractionTable(), casaScenario_, this)},
  objectiveFunctionController_{new ObjectiveFunctionControllerSAC(sacTab->objectiveFunctionTable(), casaScenario_, this)}
{
  sacTab_->lineEditProject3D()->setText("");
  sacTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

  connect( parent, SIGNAL(signalReload1Ddata()), this, SLOT(slotExtractData()));

  connect(sacTab_->pushButtonSACrunCASA(),  SIGNAL(clicked()),    this, SLOT(slotPushButtonSACrunCasaClicked()));
  connect(sacTab_->pushSelectProject3D(),   SIGNAL(clicked()),    this, SLOT(slotPushSelectProject3dClicked()));  
  connect(sacTab_->pushSelectAllWells(), SIGNAL(clicked()),       calibrationTargetController_, SLOT(slotSelectAllWells()));
  connect(sacTab_->pushClearSelection(), SIGNAL(clicked()),       calibrationTargetController_, SLOT(slotClearWellSelection()));

  connect(sacTab_->buttonRunOriginal1D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal1D()));
  connect(sacTab_->buttonRunOriginal3D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal3D()));

  connect(sacTab_->comboBoxCluster(),       SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxClusterCurrentTextChanged(QString)));
  connect(sacTab_->comboBoxApplication(),   SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxApplicationChanged(QString)));
}

void SACcontroller::refreshGUI()
{
  sacTab_->lineEditProject3D()->setText(casaScenario_.project3dPath());  
  sacTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

  if (casaScenario_.applicationName() == "fastcauldron \"-itcoupled\"")
  {
    sacTab_->comboBoxApplication()->setCurrentText("Iteratively Coupled");
  }
  else if (casaScenario_.applicationName() == "fastcauldron \"-temperature\"")
  {
    sacTab_->comboBoxApplication()->setCurrentText("Hydrostatic");
  }

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

void SACcontroller::slotExtractData()
{
  if (casaScenario_.project3dPath().isEmpty())
  {
    return;
  }

  dataExtractionController_->readOriginalResults();
  dataExtractionController_->readOptimizedResults();
  scenarioBackup::backup(casaScenario_);
}

void SACcontroller::slotPushButtonSACrunCasaClicked()
{
  wellTrajectoryWriter::writeTrajectories(casaScenario_);

  const QString workingDir = casaScenario_.workingDirectory();
  const QString calibrationDir = casaScenario_.calibrationDirectory();

  if (workingDir.isEmpty())
  {
    return;
  }

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

  const bool filesCopied = functions::copyCaseFolder(QDir(workingDir), QDir(calibrationDir));

  Logger::log() << (filesCopied ? "Finished copying case to " + calibrationDir :
                                  "Failed copying case, no files were copied") << Logger::endl();

  CMBProjectWriter projectWriter(calibrationDir + "/" + casaScenario_.project3dFilename());
  casaScenario_.updateRelevantProperties(projectWriter);

  scenarioBackup::backup(casaScenario_);
  SACScript sac{casaScenario_, calibrationDir};
  if (!casaScriptWriter::writeCasaScript(sac))
  {
    return;
  }

  if (!scriptRunController_.runScript(sac))
  {
    QDir(calibrationDir + "/" + casaScenario_.runLocation()).removeRecursively();
    return;
  }

  dataExtractionController_->readOptimizedResults();

  if (QFile::copy(calibrationDir + "/" + casaScenario_.stateFileNameSAC() ,
                  calibrationDir + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameSAC()))
  {
    QFile file (calibrationDir + "/" + casaScenario_.stateFileNameSAC());
    file.remove();
    Logger::log() << "Done!" << Logger::endl();
  }
  else
  {
    Logger::log() << "- An error occurred while moving the state file to the last iteration folder." << Logger::endl();
  }
  scenarioBackup::backup(casaScenario_);
}

void SACcontroller::renameCaseFolders(const QString& pathName)
{
  const CalibrationTargetManager& ctManager = casaScenario_.calibrationTargetManager();
  const QVector<const Well*>& wells = ctManager.wells();
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(wells);

  int caseIndex = 0;
  for (const int wellIndex : sortedIndices)
  {
    if (wells[wellIndex]->isActive())
    {
      QString oldFolderName = pathName + "Case_" + QString::number(++caseIndex);
      QString newFolderName = pathName + wells[wellIndex]->name();
      QDir(oldFolderName).rename(oldFolderName, newFolderName);
    }
  }
}

void SACcontroller::slotRunOriginal1D()
{
  wellTrajectoryWriter::writeTrajectories(casaScenario_);

  const QString workingDir = casaScenario_.workingDirectory();
  const QString original1dDir{casaScenario_.original1dDirectory()};

  if (workingDir.isEmpty())
  {
    return;
  }

  QDir calDir(original1dDir);
  if (calDir.exists())
  {
    QDir(original1dDir).removeRecursively();
    calDir.mkpath(".");
  }
  else
  {
    calDir.mkpath(".");
  }

  const bool filesCopied = functions::copyCaseFolder(QDir(workingDir), QDir(original1dDir));

  Logger::log() << (filesCopied ? "Finished copying case to " + original1dDir :
                                  "Failed copying case, no files were copied") << Logger::endl();

  CMBProjectWriter projectWriter(original1dDir + "/" + casaScenario_.project3dFilename());
  casaScenario_.updateRelevantProperties(projectWriter);

  scenarioBackup::backup(casaScenario_);
  SACScript sac(casaScenario_, original1dDir, false);
  if (!casaScriptWriter::writeCasaScript(sac))
  {
    return;
  }

  if (!scriptRunController_.runScript(sac))
  {
    QDir(original1dDir).removeRecursively();
    return;
  }

  renameCaseFolders(casaScenario_.original1dDirectory() + "/" + casaScenario_.runLocation() + "/Iteration_1/");
  dataExtractionController_->readOriginalResults();

  if (QFile::copy(casaScenario_.original1dDirectory() + "/" + casaScenario_.stateFileNameSAC() ,
                  casaScenario_.original1dDirectory() + "/" + casaScenario_.runLocation() + "/Iteration_1/" + casaScenario_.stateFileNameSAC()))
  {
    QFile file (casaScenario_.original1dDirectory() + "/" + casaScenario_.stateFileNameSAC());
    file.remove();
    Logger::log() << "Done!" << Logger::endl();
  }
  else
  {
    Logger::log() << "- An error occurred while moving the state file to the iteration folder." << Logger::endl();
  }
  scenarioBackup::backup(casaScenario_);
}

void SACcontroller::slotRunOriginal3D()
{
  if (casaScenario_.project3dPath().isEmpty())
  {
    return;
  }

  const QString runDirectory{casaScenario_.calibrationDirectory() + "/ThreeDBase"};

  const QDir sourceDir(casaScenario_.workingDirectory());
  const QDir targetDir(runDirectory);
  if (!sourceDir.exists())
  {
    Logger::log() << "Source directory " + sourceDir.absolutePath() + " not found" << Logger::endl();
    return;
  } 

  const bool filesCopied = functions::copyCaseFolder(sourceDir, targetDir);
  if (!filesCopied)
  {
    Logger::log() << "Failed to create the 3D original case"
                  << "\nThe original is not run" << Logger::endl();
    return;
  }
  CMBProjectWriter projectWriter(runDirectory + "/" + casaScenario_.project3dFilename());
  casaScenario_.updateRelevantProperties(projectWriter);

  Run3dCaseController run3dCaseController(casaScenario_, scriptRunController_);
  if (run3dCaseController.run3dCase(runDirectory, false))
  {
    scenarioBackup::backup(casaScenario_);
  }
}

void SACcontroller::slotPushSelectProject3dClicked()
{
  const QString fileName = QFileDialog::getOpenFileName(sacTab_,
                                                        "Select project file",
                                                        casaScenario_.project3dPath(),
                                                        "Project files (*.project3d)");
  if (fileName == "")
  {
    return;
  }

  const QDir fileNamePath = QFileInfo(fileName).absoluteDir();
  const QString originalWorkspaceLocation = fileNamePath.absolutePath();

  CMBProjectReader tempProjectReader;
  tempProjectReader.load(fileName);
  if (!tempProjectReader.hasDepthDefinedInAllLayers())
  {
    QMessageBox box(QMessageBox::Critical, "Project file not accepted!",
                    "Thickness defined layers are not supported in the SAC workflow, please convert to depth maps prior to input",
                    QMessageBox::Ok);
    box.exec();
    return;
  }

  WorkspaceDialog popupWorkspace{originalWorkspaceLocation, casaWizard::workspaceGenerator::getSuggestedWorkspace(fileName)};
  if (popupWorkspace.exec() != QDialog::Accepted)
  {
    return;
  }

  const QString workingDirectory = popupWorkspace.optionSelected();
  if (!functions::overwriteIfDirectoryExists(workingDirectory))
  {
    return;
  }

  if (!casaWizard::workspaceGenerator::createWorkspace(originalWorkspaceLocation, workingDirectory))
  {
    Logger::log() << "Unable to create workspace, do you have write access to: " << workingDirectory << Logger::endl();
    return;
  }

  casaScenario_.clear();
  casaScenario_.setWorkingDirectory(workingDirectory);  
  casaScenario_.setProject3dFilePath(fileName);  
  casaScenario_.updateT2zLastSurface();
  lithofractionController_->loadLayersFromProject();  

  scenarioBackup::backup(casaScenario_);
  refreshGUI();
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
