#include "doeController.h"

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
#include "model/output/runCaseSetFileManager.h"
#include "model/output/workspaceGenerator.h"
#include "view/doeTab.h"
#include "view/uaTabIDs.h"
#include "view/workspaceDialog.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QListWidget>
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

DoEcontroller::DoEcontroller(DoeTab* doeTab,
                             UAScenario& casaScenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent) :
  QObject(parent),
  doeTab_{doeTab},
  casaScenario_{casaScenario},
  scriptRunController_{scriptRunController},
  influentialParameterController_{new InfluentialParameterController(doeTab_->influentialParameterTable(), casaScenario_.influentialParameterManager(), this)},
  manualDesignPointController_{new ManualDesignPointController(doeTab_->manualDesignPointTable(), casaScenario_.manualDesignPointManager(), this)}
{
  doeTab_->spinBoxCPUs()->setValue(casaScenario_.numberCPUs());
  doeTab_->comboBoxApplication()->setCurrentText(casaScenario_.applicationName());
  doeTab_->lineEditProject3D()->setText("");
  doeTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

  slotUpdateDoeOptionTable();

  connect(doeTab_->pushButtonDoeRunCASA(),    SIGNAL(clicked()),                          this, SLOT(slotPushButtonDoErunCasaClicked()));
  connect(doeTab_->pushButtonRunAddedCases(), SIGNAL(clicked()),                          this, SLOT(slotPushButtonRunAddedCasesClicked()));
  connect(doeTab_->pushSelectProject3D(),     SIGNAL(clicked()),                          this, SLOT(slotPushSelectProject3dClicked()));
  connect(doeTab_->lineEditProject3D(),       SIGNAL(textChanged(const QString&)),        this, SLOT(slotLineEditProject3dTextChanged(const QString&)));
  connect(doeTab_->comboBoxCluster(),         SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotComboBoxClusterCurrentTextChanged(const QString&)));
  connect(doeTab_->comboBoxApplication(),     SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotComboBoxApplicationCurrentTextChanged(const QString&)));
  connect(doeTab_->spinBoxCPUs(),             SIGNAL(valueChanged(int)),                  this, SLOT(slotSpinBoxCpusValueChanged(int)));
  connect(doeTab_->doeOptionTable(),          SIGNAL(itemChanged(QTableWidgetItem*)) ,    this, SLOT(slotDoeSelectionItemChanged(QTableWidgetItem*)));

  connect(influentialParameterController_,    SIGNAL(influentialParametersChanged()),     this, SLOT(slotUpdateDoeOptionTable()));
  connect(influentialParameterController_,    SIGNAL(influentialParametersChanged()),     this, SLOT(slotUpdateDesignPointTable()));
  connect(influentialParameterController_,    SIGNAL(removeInfluentialParameter(int)),
          manualDesignPointController_,       SLOT(removeParameter(int)));
  connect(manualDesignPointController_,       SIGNAL(designPointsChanged()),              this, SLOT(slotManualDesignPointsChanged()));

  connect(this, SIGNAL(signalRefresh()), influentialParameterController_, SLOT(slotRefresh()));
  connect(this, SIGNAL(signalRefresh()), manualDesignPointController_,    SLOT(slotRefresh()));
}

void DoEcontroller::slotUpdateIterationDir()
{
  RunCaseSetFileManager& runCaseSetFileManager = casaScenario_.runCaseSetFileManager();
  runCaseSetFileManager.setIterationPath(casaScenario_.project3dPath());
}

void DoEcontroller::setDoEstageIncomplete()
{
  casaScenario_.setStageComplete(StageTypesUA::doe, false);
  casaScenario_.setStageComplete(StageTypesUA::qc, false);
  casaScenario_.setStageComplete(StageTypesUA::mcmc, false);
}

void DoEcontroller::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::DoE))
  {
    return;
  }

  doeTab_->setEnabled(true);
  if (casaScenario_.isStageComplete(StageTypesUA::doe))
  {
    const RunCaseSetFileManager& runCaseSetFileManager = casaScenario_.runCaseSetFileManager();
    if (runCaseSetFileManager.isIterationDirDeleted(casaScenario_.project3dPath()))
    {
      doeTab_->setEnabled(false);
      Logger::log() << "Disabled DoE tab!" << Logger::endl();
    }
  }
  else
  {
    Logger::log() << "DoE stage is not completed! Run CASA to complete it." << Logger::endl();
  }

  slotRefresh();
}

void DoEcontroller::slotRefresh()
{
  QSignalBlocker blockerProject3d(doeTab_->lineEditProject3D());
  doeTab_->lineEditProject3D()->setText(casaScenario_.project3dPath());

  QSignalBlocker blockerApplication(doeTab_->comboBoxApplication());
  doeTab_->comboBoxApplication()->setCurrentText(casaScenario_.applicationName());

  QSignalBlocker blockerCluster(doeTab_->comboBoxCluster());
  doeTab_->comboBoxCluster()->setCurrentText(casaScenario_.clusterName());

  QSignalBlocker blockerCPUs(doeTab_->spinBoxCPUs());
  doeTab_->spinBoxCPUs()->setValue(casaScenario_.numberCPUs());

  QSignalBlocker blockerDoeOptionTable(doeTab_->doeOptionTable());
  doeTab_->updateDoeOptionTable(casaScenario_.doeOptions(), casaScenario_.isDoeOptionSelected());

  emit signalRefresh();
}

void DoEcontroller::slotUpdateDoeOptionTable()
{
  const InfluentialParameterManager& manager = casaScenario_.influentialParameterManager();
  casaScenario_.updateDoeConstantNumberOfDesignPoints(manager.totalNumberOfInfluentialParameters());
  doeTab_->updateDoeOptionTable(casaScenario_.doeOptions(), casaScenario_.isDoeOptionSelected());

  setDoEstageIncomplete();
}

void DoEcontroller::slotUpdateDesignPointTable()
{
  const InfluentialParameterManager& influentialParameterManager = casaScenario_.influentialParameterManager();

  const int numberNew = influentialParameterManager.influentialParameters().size();
  const QStringList names = influentialParameterManager.nameList();

  manualDesignPointController_->updateInfluentialParameters(numberNew, names);

  setDoEstageIncomplete();
}

void DoEcontroller::slotPushButtonDoErunCasaClicked()
{
  if (casaScenario_.isStageComplete(StageTypesUA::doe))
  {
    Logger::log() << "Nothing to be done. DoE is already run with current settings" << Logger::endl();
    return;
  }

  scenarioBackup::backup(casaScenario_);
  ManualDesignPointManager& designPointManager = casaScenario_.manualDesignPointManager();
  if (casaScenario_.doeSelected().isEmpty() && designPointManager.numberOfPoints() == 0)
  {
    Logger::log() << "Nothing to be done. At least one doe must be selected (non selected) or one design point must be added" << Logger::endl();
    return;
  }

  DoEScript doe{casaScenario_};
  if (!casaScriptWriter::writeCasaScript(doe))
  {
    return;
  }
  scriptRunController_.runScript(doe);
  designPointManager.completeAll();

  casaScenario_.setStageComplete(StageTypesUA::doe, true);

  const QString source = casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameDoE();
  const QString target = casaScenario_.workingDirectory() + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameDoE();

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

  scenarioBackup::backup(casaScenario_);
}

void DoEcontroller::slotPushButtonRunAddedCasesClicked()
{
  if (casaScenario_.isStageComplete(StageTypesUA::doe))
  {
    Logger::log() << "Nothing to be done. DoE is already run with current settings" << Logger::endl();
    return;
  }

  scenarioBackup::backup(casaScenario_);
  ManualDesignPointManager& designPointManager = casaScenario_.manualDesignPointManager();
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

  AddCasesScript addCasesScript{casaScenario_};
  RunCaseSetFileManager& rcsFileManager = casaScenario_.runCaseSetFileManager();
  if (!casaScriptWriter::writeCasaScriptFilterOutDataDir(addCasesScript, rcsFileManager.caseSetDirPath()))
  {
    return;
  }

  scriptRunController_.runScript(addCasesScript);
  designPointManager.completeAll();

  casaScenario_.setStageComplete(StageTypesUA::doe);

  slotUpdateIterationDir();
  slotUpdateTabGUI(static_cast<int>(TabID::DoE));

  scenarioBackup::backup(casaScenario_);
}

void DoEcontroller::slotPushSelectProject3dClicked()
{
  QString fileName = QFileDialog::getOpenFileName(doeTab_,
                                                  "Select project file",
                                                  casaScenario_.workingDirectory(),
                                                  "Project files (*.project3d *.project1d)");
  if (fileName == "")
  {
    return;
  }

  const QDir fileNamePath = QFileInfo(fileName).absoluteDir();
  const QString originalWorkspaceLocation = fileNamePath.absolutePath();

  WorkspaceDialog popupWorkspace{originalWorkspaceLocation,casaWizard::workspaceGenerator::getSuggestedWorkspace(fileName) };

  if (popupWorkspace.exec() != QDialog::Accepted)
  {
    return;
  }

  if (!casaWizard::workspaceGenerator::createWorkspace(originalWorkspaceLocation, popupWorkspace.optionSelected()))
  {
    Logger::log() << "Unable to create workspace, do you have write access to: " << popupWorkspace.optionSelected() << Logger::endl();
    return;
  }

  setDoEstageIncomplete();

  casaScenario_.setWorkingDirectory(popupWorkspace.optionSelected());
  casaScenario_.setProject3dFilePath(fileName);

  const QString workingDir = casaScenario_.workingDirectory();
  const QString projectName = casaScenario_.project3dFilename();

  doeTab_->lineEditProject3D()->setText(workingDir + "/" + projectName);
}

void DoEcontroller::slotComboBoxApplicationCurrentTextChanged(const QString& applicationName)
{
  setDoEstageIncomplete();
  casaScenario_.setApplicationName(applicationName);
}

void DoEcontroller::slotLineEditProject3dTextChanged(const QString& project3dPath)
{
  setDoEstageIncomplete();
  casaScenario_.setProject3dFilePath(project3dPath);
}

void DoEcontroller::slotSpinBoxCpusValueChanged(int cpus)
{
  casaScenario_.setNumberCPUs(cpus);
}

void DoEcontroller::slotManualDesignPointsChanged()
{
  setDoEstageIncomplete();
  casaScenario_.setNumberOfManualDesignPoints();
  slotRefresh();
}

void DoEcontroller::slotComboBoxClusterCurrentTextChanged(const QString& clusterName)
{
  casaScenario_.setClusterName(clusterName);
}

void DoEcontroller::slotDoeSelectionItemChanged(QTableWidgetItem* item)
{
  setDoEstageIncomplete();
  if (item->column() == doeTab_->columnIndexCheckBoxDoeOptionTable())
  {
    casaScenario_.setIsDoeOptionSelected(item->row(), item->checkState() == Qt::Checked);
  }
  else if (item->column() == doeTab_->columnIndexNDesignPointsDoeOptionTable() && !casaScenario_.doeOptions()[item->row()]->hasCalculatedDesignPoints())
  {
    casaScenario_.updateDoeArbitraryNDesignPoints(item->row(), item->data(0).toInt());
  }
}

} // namespace ua

} // namespace casaWizard
