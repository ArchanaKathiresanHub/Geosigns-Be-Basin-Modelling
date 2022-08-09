#include "ModelInputsController.h"

#include "control/functions/folderOperations.h"
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

   connect(parent, SIGNAL(signalProjectOpened()),   this, SLOT(slotUpdateIterationDir()));
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

void ModelInputsController::slotUpdateIterationDir()
{
   RunCaseSetFileManager& runCaseSetFileManager = m_casaScenario.runCaseSetFileManager();
   runCaseSetFileManager.setIterationPath(m_casaScenario.project3dPath());
}

void ModelInputsController::setDoEstageIncomplete()
{
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
      const RunCaseSetFileManager& runCaseSetFileManager = m_casaScenario.runCaseSetFileManager();
      if (runCaseSetFileManager.isIterationDirDeleted(m_casaScenario.project3dPath()))
      {
         m_modelInputsTab->setEnabled(false);
         Logger::log() << "Disabled DoE tab!" << Logger::endl();
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
   return m_casaScenario.isStageComplete(StageTypesUA::doe) ||  designPointManager.numberOfCasesToRun() == 0;
}

bool ModelInputsController::buttonDoERunShouldBeDisabled()
{
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();
   return m_casaScenario.isStageComplete(StageTypesUA::doe) || (m_casaScenario.doeSelected().isEmpty() && designPointManager.numberOfPoints() == 0);
}

void ModelInputsController::slotUpdateDoeOptionTable()
{
   const InfluentialParameterManager& manager = m_casaScenario.influentialParameterManager();
   m_casaScenario.updateDoeConstantNumberOfDesignPoints(manager.totalNumberOfInfluentialParameters());
   m_modelInputsTab->updateDoeOptionTable(m_casaScenario.doeOptions(), m_casaScenario.isDoeOptionSelected());

   setDoEstageIncomplete();
}

void ModelInputsController::slotUpdateDesignPointTable()
{
   const InfluentialParameterManager& influentialParameterManager = m_casaScenario.influentialParameterManager();

   const int numberNew = influentialParameterManager.influentialParameters().size();
   const QStringList names = influentialParameterManager.nameList();

   m_manualDesignPointController->updateInfluentialParameters(numberNew, names);

   setDoEstageIncomplete();
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
   if (m_casaScenario.doeSelected().isEmpty() && designPointManager.numberOfPoints() == 0)
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
   designPointManager.completeAll();

   m_casaScenario.setStageComplete(StageTypesUA::doe, true);

   const QString source = m_casaScenario.workingDirectory() + "/" + m_casaScenario.stateFileNameDoE();
   const QString target = m_casaScenario.workingDirectory() + "/" + m_casaScenario.runLocation() + "/" + m_casaScenario.iterationDirName() + "/" + m_casaScenario.stateFileNameDoE();

   if (QFile::exists(target))
   {
      QFile::remove(target);
   }

   if (QFile::copy(source, target))
   {
      QFile::remove(source);
   }

   slotUpdateIterationDir();
   slotUpdateTabGUI(static_cast<int>(TabID::DoE));

   scenarioBackup::backup(m_casaScenario);
}

void ModelInputsController::slotPushButtonRunAddedCasesClicked()
{
   if (m_casaScenario.isStageComplete(StageTypesUA::doe))
   {
      Logger::log() << "Nothing to be done. DoE is already run with current settings" << Logger::endl();
      return;
   }

   scenarioBackup::backup(m_casaScenario);
   ManualDesignPointManager& designPointManager = m_casaScenario.manualDesignPointManager();
   QVector<bool> completed = designPointManager.completed();

   int numberToRun{0};
   for (const bool b : completed)
   {
      numberToRun += b ? 0 : 1;
   }
   if (numberToRun == 0)
   {
      Logger::log() << "There are no manual design points to be run." << Logger::endl();
      return;
   }

   AddCasesScript addCasesScript{m_casaScenario};
   RunCaseSetFileManager& rcsFileManager = m_casaScenario.runCaseSetFileManager();
   if (!casaScriptWriter::writeCasaScriptFilterOutDataDir(addCasesScript, rcsFileManager.caseSetDirPath()) ||
       !m_scriptRunController.runScript(addCasesScript))
   {
      return;
   }
   designPointManager.completeAll();

   m_casaScenario.setStageComplete(StageTypesUA::doe);

   slotUpdateIterationDir();
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

   setDoEstageIncomplete();

   m_casaScenario.setWorkingDirectory(popupWorkspace.optionSelected());
   m_casaScenario.setProject3dFileNameAndLoadFile(fileName);

   const QString workingDir = m_casaScenario.workingDirectory();
   const QString projectName = m_casaScenario.project3dFilename();

   m_modelInputsTab->lineEditProject3D()->setText(workingDir + "/" + projectName);
   refreshGUI();
}

void ModelInputsController::slotComboBoxApplicationCurrentTextChanged(const QString& applicationName)
{
   setDoEstageIncomplete();
   m_casaScenario.setApplicationName(applicationName);
   refreshGUI();
}

void ModelInputsController::slotLineEditProject3dTextChanged(const QString& project3dPath)
{
   setDoEstageIncomplete();
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
   setDoEstageIncomplete();
   refreshGUI();
}

void ModelInputsController::slotManualDesignPointsChanged()
{
   setDoEstageIncomplete();
   m_casaScenario.setNumberOfManualDesignPoints();
   refreshGUI();
}

void ModelInputsController::slotComboBoxClusterCurrentTextChanged(const QString& clusterName)
{
   m_casaScenario.setClusterName(clusterName);
}

void ModelInputsController::slotDoeSelectionItemChanged(QTableWidgetItem* item)
{
   setDoEstageIncomplete();
   if (item->column() == m_modelInputsTab->columnIndexCheckBoxDoeOptionTable())
   {
      m_casaScenario.setIsDoeOptionSelected(item->row(), item->checkState() == Qt::Checked);
   }
   else if (item->column() == m_modelInputsTab->columnIndexNDesignPointsDoeOptionTable() && !m_casaScenario.doeOptions()[item->row()]->hasCalculatedDesignPoints())
   {
      m_casaScenario.updateDoeArbitraryNDesignPoints(item->row(), item->data(0).toInt());
   }

   refreshGUI();
}

} // namespace ua

} // namespace casaWizard
