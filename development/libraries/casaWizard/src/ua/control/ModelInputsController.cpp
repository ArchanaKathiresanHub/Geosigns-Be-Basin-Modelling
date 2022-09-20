//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ModelInputsController.h"

#include "model/functions/folderOperations.h"
#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "influentialParameterController.h"
#include "manualDesignPointController.h"
#include "model/doeOption.h"
#include "model/logger.h"
#include "model/scenarioBackup.h"
#include "model/script/addCasesScript.h"
#include "model/script/doeScript.h"
#include "model/stagesUA.h"
#include "model/uaScenario.h"
#include "model/input/dataFileParser.h"
#include "model/output/cmbProjectWriter.h"
#include "model/output/runCaseSetFileManager.h"
#include "model/output/workspaceGenerator.h"
#include "view/ModelInputsTab.h"
#include "view/uaTabIDs.h"
#include "view/workspaceDialog.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>

#include <assert.h>

namespace casaWizard
{

namespace ua
{

ModelInputsController::ModelInputsController(ModelInputsTab* modelInputsTab,
                                             UAScenario& casaScenario,
                                             ScriptRunController& scriptRunController,
                                             QObject* parent) :
   QObject(parent),
   m_modelInputsTab{modelInputsTab},
   m_casaScenario{casaScenario},
   m_scriptRunController{scriptRunController},
   m_influentialParameterController{new InfluentialParameterController(m_modelInputsTab->influentialParameterTable(), m_casaScenario.influentialParameterManager(), this)},
   m_manualDesignPointController{new ManualDesignPointController(m_modelInputsTab->manualDesignPointTable(), m_casaScenario.manualDesignPointManager(), this)}
{
   m_modelInputsTab->spinBoxCPUs()->setValue(m_casaScenario.numberCPUs());
   m_modelInputsTab->comboBoxApplication()->setCurrentText(m_casaScenario.applicationName());
   m_modelInputsTab->lineEditProject3D()->setText("");
   m_modelInputsTab->comboBoxCluster()->setCurrentText(m_casaScenario.clusterName());

   slotUpdateDoeOptionTable();

   connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

   connect(m_modelInputsTab->pushButtonDoeRunCASA(),    SIGNAL(clicked()),                          this, SLOT(slotPushButtonDoErunCasaClicked()));
   connect(m_modelInputsTab->pushButtonRunAddedCases(), SIGNAL(clicked()),                          this, SLOT(slotPushButtonRunAddedCasesClicked()));
   connect(m_modelInputsTab->pushSelectProject3D(),     SIGNAL(clicked()),                          this, SLOT(slotPushSelectProject3dClicked()));
   connect(m_modelInputsTab->lineEditProject3D(),       SIGNAL(textChanged(const QString&)),        this, SLOT(slotLineEditProject3dTextChanged(const QString&)));
   connect(m_modelInputsTab->comboBoxCluster(),         SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotComboBoxClusterCurrentTextChanged(const QString&)));
   connect(m_modelInputsTab->comboBoxApplication(),     SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotComboBoxApplicationCurrentTextChanged(const QString&)));
   connect(m_modelInputsTab->spinBoxCPUs(),             SIGNAL(valueChanged(int)),                  this, SLOT(slotSpinBoxCpusValueChanged(int)));
   connect(m_modelInputsTab->spinBoxSubSampling(),      SIGNAL(valueChanged(int)),                  this, SLOT(slotSpinBoxSubSamplingValueChanged(int)));
   connect(m_modelInputsTab->doeOptionTable(),          SIGNAL(itemChanged(QTableWidgetItem*)) ,    this, SLOT(slotDoeSelectionItemChanged(QTableWidgetItem*)));

   connect(m_influentialParameterController,    SIGNAL(influentialParametersChanged()),     this, SLOT(slotUpdateDoeOptionTable()));
   connect(m_influentialParameterController,    SIGNAL(influentialParametersChanged()),     this, SLOT(slotUpdateDesignPointTable()));
   connect(m_influentialParameterController,    SIGNAL(removeInfluentialParameter(int)),
           m_manualDesignPointController,       SLOT(removeParameter(int)));
   connect(m_manualDesignPointController,       SIGNAL(designPointsChanged()),              this, SLOT(slotManualDesignPointsChanged()));
}

void ModelInputsController::resetDoEStage()
{
   //When re-performing the DoE stage, the old casa state is no longer used, thus the manual design points should all be unlocked.
   m_casaScenario.manualDesignPointManager().setAllIncomplete();
   m_casaScenario.manualDesignPointManager().removeHiddenPoints();
   m_casaScenario.setStageComplete(StageTypesUA::doe, false);
   m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces, false);
   m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);
}

void ModelInputsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::DoE))
   {
      return;
   }

   m_modelInputsTab->setEnabled(true);
   if (m_casaScenario.isStageComplete(StageTypesUA::doe))
   {
      if (!m_casaScenario.iterationDirExists())
      {
         m_casaScenario.clear();
         Logger::log() << "Error: Invalid iteration directory. Could not continue the workflow." << Logger::endl();
      }
   }
   else
   {
      Logger::log() << "DoE stage is not completed! Run CASA to complete it." << Logger::endl();
   }

   refreshGUI();
}

void ModelInputsController::refreshGUI()
{
   QSignalBlocker blockerProject3d(m_modelInputsTab->lineEditProject3D());
   m_modelInputsTab->lineEditProject3D()->setText(m_casaScenario.project3dPath());

   QSignalBlocker blockerApplication(m_modelInputsTab->comboBoxApplication());
   m_modelInputsTab->comboBoxApplication()->setCurrentText(m_casaScenario.applicationName());

   QSignalBlocker blockerCluster(m_modelInputsTab->comboBoxCluster());
   m_modelInputsTab->comboBoxCluster()->setCurrentText(m_casaScenario.clusterName());

   QSignalBlocker blockerCPUs(m_modelInputsTab->spinBoxCPUs());
   m_modelInputsTab->spinBoxCPUs()->setValue(m_casaScenario.numberCPUs());

   QSignalBlocker blockerSubSampling(m_modelInputsTab->spinBoxSubSampling());
   m_modelInputsTab->spinBoxSubSampling()->setValue(m_casaScenario.subSamplingFactor());

   QSignalBlocker blockerDoeOptionTable(m_modelInputsTab->doeOptionTable());
   m_modelInputsTab->updateDoeOptionTable(m_casaScenario.doeOptions(), m_casaScenario.isDoeOptionSelected());

   m_modelInputsTab->pushButtonRunAddedCases()->setDisabled(buttonRunAddedCasesShouldBeDisabled());
   m_modelInputsTab->pushButtonDoeRunCASA()->setDisabled(buttonDoERunShouldBeDisabled());

   emit signalRefreshChildWidgets();
}

bool ModelInputsController::buttonRunAddedCasesShouldBeDisabled()
{
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();
   return !m_casaScenario.isStageComplete(StageTypesUA::doe) ||  designPointManager.numberOfCasesToRun() == 0 || !doeOptionUserDefinedSelectionState();
}

bool ModelInputsController::doeOptionUserDefinedSelectionState()
{
   QVector<DoeOption*> doeOptions = m_casaScenario.doeOptions();
   QVector<bool> selectionState = m_casaScenario.isDoeOptionSelected();
   assert(doeOptions.size() == selectionState.size());

   for (int i = 0; i<doeOptions.size(); i++)
   {
      if (doeOptions[i]->name() == "UserDefined")
      {
         return selectionState[i];
      }
   }
   return false;
}

bool ModelInputsController::buttonDoERunShouldBeDisabled()
{
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();
   return m_casaScenario.isStageComplete(StageTypesUA::doe) || (m_casaScenario.doeSelected().isEmpty() && designPointManager.numberOfVisiblePoints() == 0);
}

void ModelInputsController::slotUpdateDoeOptionTable()
{
   const InfluentialParameterManager& manager = m_casaScenario.influentialParameterManager();
   m_casaScenario.updateDoeConstantNumberOfDesignPoints(manager.totalNumberOfInfluentialParameters());
   m_modelInputsTab->updateDoeOptionTable(m_casaScenario.doeOptions(), m_casaScenario.isDoeOptionSelected());

   resetDoEStage();
}

void ModelInputsController::slotUpdateDesignPointTable()
{
   const InfluentialParameterManager& influentialParameterManager = m_casaScenario.influentialParameterManager();

   const int numberNew = influentialParameterManager.influentialParameters().size();
   const QStringList names = influentialParameterManager.labelNameList();

   m_manualDesignPointController->updateInfluentialParameters(numberNew, names);

   resetDoEStage();
}

void ModelInputsController::slotPushButtonDoErunCasaClicked()
{
   if (m_casaScenario.isStageComplete(StageTypesUA::doe))
   {
      Logger::log() << "Nothing to be done. DoE is already run with current settings" << Logger::endl();
      return;
   }

   scenarioBackup::backup(m_casaScenario);
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();
   if (m_casaScenario.doeSelected().isEmpty() && designPointManager.numberOfVisiblePoints() == 0)
   {
      Logger::log() << "Nothing to be done. At least one doe must be selected (non selected) or one design point must be added" << Logger::endl();
      return;
   }

   CMBProjectWriter writer(m_casaScenario.project3dPath());
   writer.setScaling(m_casaScenario.subSamplingFactor(), m_casaScenario.subSamplingFactor());

   DoEScript doe{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(doe) ||
       !m_scriptRunController.runScript(doe))
   {
      return;
   }

   QString stateFileName = m_casaScenario.workingDirectory() + "/" + m_casaScenario.simStatesTextFileName();
   QString doeIndicesFileName = m_casaScenario.workingDirectory() + "/" + m_casaScenario.doeIndicesTextFileName();
   try
   {
      designPointManager.readAndSetCompletionStates(stateFileName, doeIndicesFileName, m_casaScenario.doeOptionSelectedNames());
   }
   catch (std::exception e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

   m_casaScenario.setStageUpToDate(StageTypesUA::doe, true);
   m_casaScenario.setStageComplete(StageTypesUA::doe, true);

   m_casaScenario.updateIterationDir();
   m_casaScenario.copyToIterationDir(m_casaScenario.stateFileNameDoE());

   slotUpdateTabGUI(static_cast<int>(TabID::DoE));
   scenarioBackup::backup(m_casaScenario);
}

void ModelInputsController::slotPushButtonRunAddedCasesClicked()
{
   if (!m_casaScenario.isStageComplete(StageTypesUA::doe))
   {
      Logger::log() << "Run DoE before running added cases." << Logger::endl();
      return;
   }

   scenarioBackup::backup(m_casaScenario);
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();

   if (designPointManager.numberOfCasesToRun() == 0)
   {
      Logger::log() << "There are no manual design points to be run." << Logger::endl();
      return;
   }

   AddCasesScript addCasesScript{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(addCasesScript) ||
       !m_scriptRunController.runScript(addCasesScript))
   {
      return;
   }

   //Following stages should be unlocked, but reset.
   m_casaScenario.setStageUpToDate(StageTypesUA::doe, true);
   m_casaScenario.setStageComplete(StageTypesUA::doe, true);
   m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces, false);
   m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);

   QString stateFileName = m_casaScenario.workingDirectory() + "/" + m_casaScenario.simStatesTextFileName();
   QString doeIndicesFileName = m_casaScenario.workingDirectory() + "/" + m_casaScenario.doeIndicesTextFileName();
   try
   {
      designPointManager.readAndSetCompletionStates(stateFileName, doeIndicesFileName, m_casaScenario.doeOptionSelectedNames());
   }
   catch (std::exception e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

   slotUpdateTabGUI(static_cast<int>(TabID::DoE));
   scenarioBackup::backup(m_casaScenario);
}

void ModelInputsController::slotPushSelectProject3dClicked()
{
   QString fileName = QFileDialog::getOpenFileName(m_modelInputsTab,
                                                   "Select project file",
                                                   m_casaScenario.workingDirectory(),
                                                   "Project files (*.project3d)");
   if (fileName == "")
   {
      return;
   }

   const QDir fileNamePath = QFileInfo(fileName).absoluteDir();
   const QString originalWorkspaceLocation = fileNamePath.absolutePath();

   WorkspaceDialog popupWorkspace{originalWorkspaceLocation, casaWizard::workspaceGenerator::getSuggestedWorkspace(fileName) };
   if (popupWorkspace.exec() != QDialog::Accepted)
   {
      return;
   }

   const QString workingDirectory = popupWorkspace.optionSelected();
   if (!functions::overwriteIfDirectoryExists(workingDirectory))
   {
      return;
   }

   if (!casaWizard::workspaceGenerator::createWorkspace(originalWorkspaceLocation, popupWorkspace.optionSelected()))
   {
      Logger::log() << "Unable to create workspace, do you have write access to: " << popupWorkspace.optionSelected() << Logger::endl();
      return;
   }

   resetDoEStage();

   m_casaScenario.setWorkingDirectory(popupWorkspace.optionSelected());
   m_casaScenario.setProject3dFileNameAndLoadFile(fileName);

   const QString workingDir = m_casaScenario.workingDirectory();
   const QString projectName = m_casaScenario.project3dFilename();

   m_modelInputsTab->lineEditProject3D()->setText(workingDir + "/" + projectName);
   m_modelInputsTab->spinBoxSubSampling()->setValue(m_casaScenario.projectReader().subSamplingFactor());
   m_casaScenario.initializeBaseSubSamplingFactor();
   refreshGUI();
}

void ModelInputsController::slotComboBoxApplicationCurrentTextChanged(const QString& applicationName)
{
   resetDoEStage();
   m_casaScenario.setApplicationName(applicationName);
   refreshGUI();
}

void ModelInputsController::slotLineEditProject3dTextChanged(const QString& project3dPath)
{
   resetDoEStage();
   m_casaScenario.setProject3dFileNameAndLoadFile(project3dPath);
   refreshGUI();
}

void ModelInputsController::slotSpinBoxCpusValueChanged(int cpus)
{
   m_casaScenario.setNumberCPUs(cpus);
}

void ModelInputsController::slotSpinBoxSubSamplingValueChanged(int subSamplingFactor)
{
   m_casaScenario.setSubSamplingFactor(subSamplingFactor);
   resetDoEStage();
   refreshGUI();
}

void ModelInputsController::slotManualDesignPointsChanged()
{
   m_casaScenario.setNumberOfManualDesignPoints();
   refreshGUI();
}

void ModelInputsController::slotComboBoxClusterCurrentTextChanged(const QString& clusterName)
{
   m_casaScenario.setClusterName(clusterName);
}

void ModelInputsController::slotDoeSelectionItemChanged(QTableWidgetItem* item)
{
   if (item->column() == m_modelInputsTab->columnIndexCheckBoxDoeOptionTable())
   {
      m_casaScenario.setIsDoeOptionSelected(item->row(), item->checkState() == Qt::Checked);
      QVector<DoeOption*> doeOptions = m_casaScenario.doeOptions();
      QVector<bool> selectionState = m_casaScenario.isDoeOptionSelected();
      if (doeOptions[item->row()]->name() != "UserDefined")
      {
         resetDoEStage();
      }
      else
      {  //Selecting or deselecting UserDefined points should only clear the following stages but should not lock them.
         m_casaScenario.setStageUpToDate(StageTypesUA::doe, false);
         m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces, false);
         m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);
      }
   }
   else if (item->column() == m_modelInputsTab->columnIndexNDesignPointsDoeOptionTable() && !m_casaScenario.doeOptions()[item->row()]->hasCalculatedDesignPoints())
   {
      m_casaScenario.updateDoeArbitraryNDesignPoints(item->row(), item->data(0).toInt());
   }

   refreshGUI();
}

} // namespace ua

} // namespace casaWizard
