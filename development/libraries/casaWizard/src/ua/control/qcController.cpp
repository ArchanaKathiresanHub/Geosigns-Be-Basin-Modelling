#include "qcController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/input/targetQCdataCreator.h"
#include "model/logger.h"
#include "model/scenarioBackup.h"
#include "model/script/qcScript.h"
#include "model/stagesUA.h"
#include "model/targetQC.h"
#include "model/uaScenario.h"
#include "qcDoeOptionController.h"
#include "view/qcTab.h"
#include "view/uaTabIDs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

QCController::QCController(QCTab* QCTab,
                           UAScenario& casaScenario,
                           ScriptRunController& scriptRunController,
                           QObject* parent) :
  QObject(parent),
  QCTab_{QCTab},
  casaScenario_{casaScenario},
  scriptRunController_{scriptRunController},
  qcDoeOptionController_{new QCDoeOptionController(QCTab_->qcDoeOptionTable(), casaScenario_, this)}
{
  connect(QCTab_->pushButtonQCrunCASA(), SIGNAL(clicked()),
          this,                          SLOT(slotPushButtonQCrunCasaClicked()));
  connect(QCTab_->tableQC(),             SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
          this,                          SLOT(slotTableQCCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));

  connect(this, SIGNAL(signalRefresh()), qcDoeOptionController_, SLOT(slotRefresh()));
}

void QCController::slotRefresh(int tabID)
{
  if (tabID != static_cast<int>(TabID::QC))
  {
    return;
  }

  emit signalRefresh();

  QCTab_->fillQCtable(casaScenario_.targetQCs());
  TargetQC emptyTarget;
  QCTab_->updateQCPlot(emptyTarget);
}

void QCController::slotEnableDisableRunCasa(int tabID, bool isEnabled)
{
  if (tabID != static_cast<int>(TabID::QC))
  {
    return;
  }

  QPushButton* qcTabPushButton = QCTab_->pushButtonQCrunCASA();
  qcTabPushButton->setEnabled(isEnabled);
}

void QCController::slotEnableDisableDependentWorkflowTabs(int tabID, bool hasLogMessage)
{
  if (tabID != static_cast<int>(TabID::QC))
  {
    return;
  }

  if (!QCTab_->isEnabled())
  {
    if (hasLogMessage)
    {
      Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
    }
    return;
  }

  const StageCompletionUA& stageCompletion = casaScenario_.isStageComplete();

  if (stageCompletion.isComplete(StageTypesUA::qc))
  {
    emit signalEnableDependentWorkflowTabs();
    return;
  }

  emit signalDisableDependentWorkflowTabs();

  if (hasLogMessage)
  {
    Logger::log() << "QC stage is not completed! Run CASA to complete it." << Logger::endl();
  }
}

void QCController::slotPushButtonQCrunCasaClicked()
{
  scenarioBackup::backup(casaScenario_);

  const CalibrationTargetManager& ctManager = casaScenario_.calibrationTargetManager();
  const PredictionTargetManager& ptManager = casaScenario_.predictionTargetManager();
  if ( (ctManager.amountOfActiveCalibrationTargets() + ptManager.amountAtAge0()) < 1)
  {
    Logger::log() << "There are no active targets. Either provide prediction targets or active/import wells. No response surfaces to calculate." << Logger::endl();
    return;
  }

  QStringList selectedDoes = casaScenario_.qcDoeOptionSelectedNames();
  if (selectedDoes.isEmpty())
  {
    Logger::log() << "No DOE selected for quality check calculation." << Logger::endl();
    return;
  }

  QCScript qc{casaScenario_};
  if (!casaScriptWriter::writeCasaScript(qc))
  {
    return;
  }

  if (scriptRunController_.runScript(qc))
  {
    try
    {
      targetQCdataCreator::readTargetQCs(casaScenario_);
      QCTab_->fillQCtable(casaScenario_.targetQCs());
    }
    catch (const std::exception& e)
    {
      Logger::log() << "Failed to read the targets for the quality check: " << e.what() << Logger::endl();
    }
  }

  if (QFile::copy(casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameQC() ,
                  casaScenario_.workingDirectory() + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameQC()))
  {
    if (!QFile::remove(casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameQC()))
    {
      Logger::log() << "There was a problem while moving " << casaScenario_.stateFileNameQC() << " file to " << casaScenario_.iterationDirName() << " Folder." << Logger::endl();
    }
  }

  StageCompletionUA& stageCompletion{casaScenario_.isStageComplete()};
  stageCompletion.setStageIsComplete(StageTypesUA::qc);
  stageCompletion.setStageIsComplete(StageTypesUA::mcmc, false);

  emit signalEnableDependentWorkflowTabs();
}

void QCController::slotTableQCCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
  if (current == nullptr)
  {
    return;
  }
  int row = current->row();

  QCTab_->updateQCPlot(casaScenario_.targetQCs()[row]);
  Logger::log() << "Displaying target " << casaScenario_.targetQCs()[row].id() << Logger::endl();
}

void QCController::slotEnableTabAndUpdateDependentWorkflowTabs()
{
  if (QCTab_->isEnabled())
  {
    return;
  }

  QCTab_->setEnabled(true);
  emit signalDisableDependentWorkflowTabs();
}

void QCController::slotDisableTabAndUpdateDependentWorkflowTabs()
{
  QCTab_->setEnabled(false);
  emit signalDisableDependentWorkflowTabs();
}

} // namespace ua

} // namespace casaWizard
