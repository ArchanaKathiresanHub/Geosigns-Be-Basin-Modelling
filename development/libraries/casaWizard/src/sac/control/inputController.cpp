//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "inputController.h"
#include "workspaceGenerationController.h"

#include "control/calibrationTargetController.h"
#include "control/casaScriptWriter.h"
#include "control/dataExtractionController.h"
#include "control/lithofractionController.h"
#include "control/objectiveFunctionControllerSAC.h"
#include "control/run3dCaseController.h"
#include "control/scriptRunController.h"
#include "model/functions/folderOperations.h"
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
#include "view/inputTab.h"
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

InputController::InputController(InputTab* inputTab,
                                 SACScenario& casaScenario,
                                 ScriptRunController& scriptRunController,
                                 QObject* parent) :
   QObject(parent),
   inputTab_{inputTab},
   casaScenario_{casaScenario},
   scriptRunController_{scriptRunController},
   calibrationTargetController_{new CalibrationTargetController(inputTab->calibrationTargetTable(), casaScenario_, this)},
   dataExtractionController_{new DataExtractionController(casaScenario_, scriptRunController_, this)},
   lithofractionController_{new LithofractionController(inputTab->lithofractionTable(), casaScenario_, this)},
   objectiveFunctionController_{new ObjectiveFunctionControllerSAC(inputTab->objectiveFunctionTable(), casaScenario_, this)}
{
   inputTab_->lineEditProject3D()->setText("");
   inputTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

   connect( parent, SIGNAL(signalReload1Ddata()), this, SLOT(slotExtractData()));

   connect(inputTab_->pushButtonSACrunCASA(),  SIGNAL(clicked()),    this, SLOT(slotPushButtonSACrunCasaClicked()));
   connect(inputTab_->pushSelectProject3D(),   SIGNAL(clicked()),    this, SLOT(slotPushSelectProject3dClicked()));
   connect(inputTab_->pushSelectAllWells(), SIGNAL(clicked()),       calibrationTargetController_, SLOT(slotSelectAllWells()));
   connect(inputTab_->pushClearSelection(), SIGNAL(clicked()),       calibrationTargetController_, SLOT(slotClearWellSelection()));

   connect(inputTab_->buttonRunOriginal1D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal1D()));
   connect(inputTab_->buttonRunOriginal3D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal3D()));

   connect(inputTab_->comboBoxCluster(),       SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxClusterCurrentTextChanged(QString)));
   connect(inputTab_->comboBoxApplication(),   SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxApplicationChanged(QString)));
}

void InputController::refreshGUI()
{
   inputTab_->lineEditProject3D()->setText(casaScenario_.project3dPath());
   inputTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

   if (casaScenario_.applicationName() == "fastcauldron \"-itcoupled\"")
   {
      inputTab_->comboBoxApplication()->setCurrentText("Iteratively Coupled");
   }
   else if (casaScenario_.applicationName() == "fastcauldron \"-temperature\"")
   {
      inputTab_->comboBoxApplication()->setCurrentText("Hydrostatic");
   }

   lithofractionController_->updateLithofractionTable();

   emit signalRefreshChildWidgets();
}

void InputController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Input))
   {
      return;
   }

   refreshGUI();
}

void InputController::slotExtractData()
{
   if (casaScenario_.project3dPath().isEmpty())
   {
      return;
   }

   dataExtractionController_->readOriginalResults();
   dataExtractionController_->readOptimizedResults();
   scenarioBackup::backup(casaScenario_);
}

void InputController::slotPushButtonSACrunCasaClicked()
{
   try
   {
      wellTrajectoryWriter::writeTrajectories(casaScenario_);
   }
   catch(const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
      return;
   }

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

void InputController::renameCaseFolders(const QString& pathName)
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

void InputController::slotRunOriginal1D()
{
   try
   {
      wellTrajectoryWriter::writeTrajectories(casaScenario_);
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

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

void InputController::slotRunOriginal3D()
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

void InputController::slotPushSelectProject3dClicked()
{
   const QString fileName = QFileDialog::getOpenFileName(inputTab_,
                                                         "Select project file",
                                                         QDir::currentPath(),
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

   if (casaScenario_.workingDirectory().isEmpty() || !casaScenario_.project3dPath().isEmpty())
   {
      casaScenario_.clear();
      if (!workspaceGenerationController::generateWorkSpace(originalWorkspaceLocation,casaScenario_))
      {
         return;
      }
   }
   else
   {
      workspaceGenerator::copyDir(originalWorkspaceLocation,casaScenario_.workingDirectory());
   }

   casaScenario_.setProject3dFileNameAndLoadFile(fileName);
   casaScenario_.updateT2zLastSurface();
   lithofractionController_->loadLayersFromProject();

   casaScenario_.updateWellsForProject3D();

   scenarioBackup::backup(casaScenario_);
   refreshGUI();
}

void InputController::slotComboBoxClusterCurrentTextChanged(QString clusterName)
{
   casaScenario_.setClusterName(clusterName);
}

void InputController::slotComboBoxApplicationChanged(QString application)
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
