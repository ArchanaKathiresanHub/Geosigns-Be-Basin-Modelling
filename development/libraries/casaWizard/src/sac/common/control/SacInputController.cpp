//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "control/SacInputController.h"
#include "control/workspaceGenerationController.h"

#include "control/calibrationTargetController.h"
#include "control/casaScriptWriter.h"
#include "assets/DataExtractionController.h"

#include "control/assets/objectiveFunctionControllerSAC.h"
#include "control/assets/run3dCaseController.h"
#include "control/ScriptRunController.h"

#include "model/functions/folderOperations.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/cmbMapReader.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/cmbProjectWriter.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/output/workspaceGenerator.h"
#include "model/SacScenario.h"
#include "model/casaScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/sacScript.h"
#include "model/functions/sortedByXWellIndices.h"
#include "view/SacInputTab.h"

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

SacInputController::SacInputController(SacInputTab* sacInputTab,
                                       SacScenario& casaScenario,
                                       ScriptRunController& scriptRunController,
                                       QObject* parent) :
   QObject(parent),
   m_scriptRunController{scriptRunController},
   m_calibrationTargetController{new CalibrationTargetController(sacInputTab->calibrationTargetTable(), casaScenario, this)},
   m_objectiveFunctionController{new ObjectiveFunctionControllerSAC(sacInputTab->objectiveFunctionTable(), casaScenario, this)},
   m_dataExtractionController{new DataExtractionController(casaScenario, m_scriptRunController, this)}
{
   sacInputTab->lineEditProject3D()->setText("");
   sacInputTab->comboBoxCluster()->setCurrentText(casaScenario.clusterName());

   connect( parent, SIGNAL(signalReload1Ddata()), this, SLOT(slotExtractData()));
   connect(sacInputTab->pushRun1DOptimalization(),  SIGNAL(clicked()),    this, SLOT(slotPushButton1DOptimalizationClicked()));
   connect(sacInputTab->pushSelectProject3D(),   SIGNAL(clicked()),    this, SLOT(slotPushButtonSelectProject3dClicked()));
   connect(sacInputTab->pushSelectAllWells(), SIGNAL(clicked()),       m_calibrationTargetController, SLOT(slotSelectAllWells()));
   connect(sacInputTab->pushClearSelection(), SIGNAL(clicked()),       m_calibrationTargetController, SLOT(slotClearWellSelection()));

   connect(sacInputTab->buttonRunOriginal1D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal1D()));
   connect(sacInputTab->buttonRunOriginal3D(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal3D()));

   connect(sacInputTab->comboBoxCluster(),       SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxClusterCurrentTextChanged(QString)));
   connect(sacInputTab->comboBoxApplication(),   SIGNAL(currentTextChanged(QString)), this, SLOT(slotComboBoxApplicationChanged(QString)));
}

void SacInputController::refreshGUI()
{
   inputTab()->lineEditProject3D()->setText(scenario().project3dPath());
   inputTab()->comboBoxCluster()->setCurrentText(scenario().clusterName());

   if (scenario().applicationName() == "fastcauldron \"-itcoupled\"")
   {
      inputTab()->comboBoxApplication()->setCurrentText("Iteratively Coupled");
   }
   else if (scenario().applicationName() == "fastcauldron \"-temperature\"")
   {
      inputTab()->comboBoxApplication()->setCurrentText("Hydrostatic");
   }

   inputTab()->setContentsActive(!(scenario().workingDirectory().isEmpty()));
}

ScriptRunController& SacInputController::scriptRunController()
{
   return m_scriptRunController;
}

CalibrationTargetController* SacInputController::calibrationTargetController()
{
   return m_calibrationTargetController;
}

DataExtractionController* SacInputController::dataExtractionController()
{
   return m_dataExtractionController;
}

void SacInputController::slotExtractData()
{
   if (scenario().project3dPath().isEmpty())
   {
      return;
   }

   m_dataExtractionController->readOriginalResults();
   readOptimizedResults();

   scenarioBackup::backup(scenario());
}

void SacInputController::slotRunOriginal1D()
{
   if (!prepareRun1D())
   {
      return;
   }

   const QString original1dDir{scenario().original1dDirectory()};

   std::unique_ptr<SACScript> sac = optimizationScript(original1dDir,false);

   if (!casaScriptWriter::writeCasaScript(*sac))
   {
      return;
   }

   if (!scriptRunController().runScript(*sac))
   {
      QDir(original1dDir).removeRecursively();
      return;
   }

   renameCaseFolders(scenario().original1dDirectory() + "/" + scenario().runLocation() + "/Iteration_1/");
   m_dataExtractionController->readOriginalResults();

   if (QFile::copy(scenario().original1dDirectory() + "/" + scenario().stateFileNameSAC() ,
                   scenario().original1dDirectory() + "/" + scenario().runLocation() + "/Iteration_1/" + scenario().stateFileNameSAC()))
   {
      QFile file (scenario().original1dDirectory() + "/" + scenario().stateFileNameSAC());
      file.remove();
      Logger::log() << "Done!" << Logger::endl();
   }
   else
   {
      Logger::log() << "- An error occurred while moving the state file to the iteration folder." << Logger::endl();
   }
   scenarioBackup::backup(scenario());
}

bool SacInputController::prepareRun1D() const
{
   try
   {
      wellTrajectoryWriter::writeTrajectories(scenario());
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

   const QString workingDir = scenario().workingDirectory();
   const QString original1dDir{scenario().original1dDirectory()};

   if (workingDir.isEmpty())
   {
      return false;
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
   if (!filesCopied)
   {
      return false;
   }

   CMBProjectWriter projectWriter(original1dDir + "/" + scenario().project3dFilename());
   scenario().updateRelevantProperties(projectWriter);

   scenarioBackup::backup(scenario());
   return true;
}

void SacInputController::slotPushButton1DOptimalizationClicked()
{
   prepareOptimizationRun();

   const QString calibrationDir = scenario().calibrationDirectory();
   std::unique_ptr<SACScript> sac = optimizationScript(calibrationDir);
   if (!casaScriptWriter::writeCasaScript(*sac))
   {
      return;
   }

   if (!scriptRunController().runScript(*sac))
   {
      QDir(calibrationDir + "/" + scenario().runLocation()).removeRecursively();
      return;
   }

   readOptimizedResults();

   if (QFile::copy(calibrationDir + "/" + scenario().stateFileNameSAC() ,
                   calibrationDir + "/" + scenario().runLocation() + "/" + scenario().iterationDirName() + "/" + scenario().stateFileNameSAC()))
   {
      QFile file (calibrationDir + "/" + scenario().stateFileNameSAC());
      file.remove();
      Logger::log() << "Done!" << Logger::endl();
   }
   else
   {
      Logger::log() << "- An error occurred while moving the state file to the last iteration folder." << Logger::endl();
   }
   scenarioBackup::backup(scenario());
}

void SacInputController::prepareOptimizationRun() const
{
   try
   {
      wellTrajectoryWriter::writeTrajectories(scenario());
   }
   catch(const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
      return;
   }

   const QString workingDir = scenario().workingDirectory();
   const QString calibrationDir = scenario().calibrationDirectory();

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

   CMBProjectWriter projectWriter(calibrationDir + "/" + scenario().project3dFilename());
   scenario().updateRelevantProperties(projectWriter);

   scenarioBackup::backup(scenario());
}

void SacInputController::renameCaseFolders(const QString& pathName)
{
   const CalibrationTargetManager& ctManager = scenario().calibrationTargetManager();
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

bool SacInputController::selectWorkspace()
{
   const QString fileName = QFileDialog::getOpenFileName(inputTab(),
                                                         "Select project file",
                                                         QDir::currentPath(),
                                                         "Project files (*.project3d)");
   if (fileName == "")
   {
      return false;
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
      return false;
   }

   if (scenario().workingDirectory().isEmpty() || !scenario().project3dPath().isEmpty())
   {
      scenario().clear();
      if (!workspaceGenerationController::generateWorkSpace(originalWorkspaceLocation, scenario()))
      {
         return false;
      }  
   }
   else
   {
      workspaceGenerator::copyDir(originalWorkspaceLocation,scenario().workingDirectory());
   }

   scenario().setProject3dFileNameAndLoadFile(fileName);
   scenario().updateWellsForProject3D();
   scenarioBackup::backup(scenario());
   return true;
}

void SacInputController::slotRunOriginal3D()
{
   if (scenario().project3dPath().isEmpty())
   {
      return;
   }

   const QString runDirectory{scenario().calibrationDirectory() + "/ThreeDBase"};

   const QDir sourceDir(scenario().workingDirectory());
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
   CMBProjectWriter projectWriter(runDirectory + "/" + scenario().project3dFilename());
   scenario().updateRelevantProperties(projectWriter);

   Run3dCaseController run3dCaseController(scenario(), scriptRunController());
   if (run3dCaseController.run3dCase(runDirectory, false))
   {
      scenarioBackup::backup(scenario());
   }
}

void SacInputController::slotComboBoxClusterCurrentTextChanged(QString clusterName)
{
   scenario().setClusterName(clusterName);
}

void SacInputController::slotComboBoxApplicationChanged(QString application)
{
   if (application == "Iteratively Coupled")
   {
      scenario().setApplicationName("fastcauldron \"-itcoupled\"");
   }
   else if (application == "Hydrostatic")
   {
      scenario().setApplicationName("fastcauldron \"-temperature\"");
   }
   else
   {
      scenario().setApplicationName("");
   }
}

} // namespace sac

} // namespace casaWizard
