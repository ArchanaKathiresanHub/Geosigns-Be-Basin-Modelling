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
  connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

  connect(QCTab_->pushButtonQCrunCASA(), SIGNAL(clicked()),
          this,                          SLOT(slotPushButtonQCrunCasaClicked()));
  connect(QCTab_->tableQC(),             SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
          this,                          SLOT(slotTableQCCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));

  connect(qcDoeOptionController_, SIGNAL(modelChange()), this, SLOT(slotModelChange()));
}

void QCController::slotModelChange()
{
  casaScenario_.setStageComplete(StageTypesUA::qc, false);
  casaScenario_.setStageComplete(StageTypesUA::mcmc, false);

  casaScenario_.setTargetQCs({});
  casaScenario_.monteCarloDataManager().clear();

  refreshGUI();
}

void QCController::refreshGUI()
{
  QCTab_->fillQCtable(casaScenario_.targetQCs());
  QCTab_->updateQCPlot({});

  emit signalRefreshChildWidgets();
}

void QCController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::QC))
  {
    return;
  }

  QCTab_->allowModification();
  QCTab_->setEnabled(true);

  if (!casaScenario_.isStageComplete(StageTypesUA::doe))
  {
    QCTab_->setEnabled(false);
    Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
  }
  else if (!casaScenario_.isStageComplete(StageTypesUA::qc))
  {
    Logger::log() << "QC stage is not completed! Run CASA to complete it." << Logger::endl();
  }
  else
  {
    const RunCaseSetFileManager& runCaseSetFileManager = casaScenario_.runCaseSetFileManager();
    if (runCaseSetFileManager.isIterationDirDeleted(casaScenario_.project3dPath()))
    {
      QCTab_->allowModification(false);
    }
  }

  refreshGUI();
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
  if (!casaScriptWriter::writeCasaScript(qc) ||
      !scriptRunController_.runScript(qc))
  {
    return;
  }

  try
  {
    targetQCdataCreator::readTargetQCs(casaScenario_);
    QCTab_->fillQCtable(casaScenario_.targetQCs());
  }
  catch (const std::exception& e)
  {
    Logger::log() << "Failed to read the targets for the quality check: " << e.what() << Logger::endl();
  }

  const QString source = casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameQC() ;
  const QString target = casaScenario_.workingDirectory() + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameQC();

  if (QFile::exists(target))
  {
    QFile::remove(target);
  }

  if (QFile::copy(source, target))
  {
    QFile::remove(source);
  }

  casaScenario_.setStageComplete(StageTypesUA::qc);
  casaScenario_.setStageComplete(StageTypesUA::mcmc, false);

  scenarioBackup::backup(casaScenario_);

  slotUpdateTabGUI(static_cast<int>(TabID::QC));
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

} // namespace ua

} // namespace casaWizard
