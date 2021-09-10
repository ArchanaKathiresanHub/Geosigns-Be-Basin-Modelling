#include "targetController.h"

#include "control/calibrationTargetController.h"
#include "control/depthTargetController.h"
#include "control/objectiveFunctionController.h"
#include "control/surfaceTargetController.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/logger.h"
#include "model/uaScenario.h"
#include "view/calibrationTargetTable.h"
#include "view/targetTab.h"
#include "view/uaTabIDs.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetController::TargetController(TargetTab* targetTab,
                                   UAScenario& scenario,
                                   QObject* parent) :
  QObject(parent),
  scenario_{scenario},
  targetTab_{targetTab},
  depthTargetController_{new DepthTargetController(targetTab_->depthTargetTable(), scenario_.predictionTargetManager(), this)},
  surfaceTargetController_{new SurfaceTargetController(targetTab_->surfaceTargetTable(), scenario_.predictionTargetManager(), this)},
  calibrationTargetController_{new CalibrationTargetController(targetTab_->calibrationTargetTable(), scenario_, this)},
  objectiveFunctionController_{new ObjectiveFunctionController(targetTab_->objectiveFunctionTable(), scenario_.calibrationTargetManager(), this)}
{
  connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

  connect(targetTab_->pushSelectCalibration(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectCalibrationClicked()));
  connect(targetTab_->lineEditCalibration(),   SIGNAL(textChanged(const QString&)), this, SLOT(slotLineEditCalibrationTextChanged(const QString&)));
}

void TargetController::refreshGUI()
{
  QSignalBlocker blockCalibration{targetTab_->lineEditCalibration()};
  targetTab_->lineEditCalibration()->setText("");

  emit signalRefreshChildWidgets();
}

void TargetController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Targets))
  {
    return;
  }

  if (scenario_.isStageComplete(StageTypesUA::doe))
  {
    const RunCaseSetFileManager& runCaseSetFileManager = scenario_.runCaseSetFileManager();
    targetTab_->setEnabled(!runCaseSetFileManager.isIterationDirDeleted(scenario_.project3dPath()));
  }
  else
  {
    targetTab_->setEnabled(false);
    Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
  }

  refreshGUI();
}

void TargetController::slotPushSelectCalibrationClicked()
{
  QString fileName = QFileDialog::getOpenFileName(targetTab_,
                                                  "Select calibration targets",
                                                  "",
                                                  "Spreadsheet (*.xlsx)");
  targetTab_->lineEditCalibration()->setText(fileName);
  emit signalRefreshChildWidgets();
}

void TargetController::slotLineEditCalibrationTextChanged(const QString& calibrationTargetsFilename)
{
  scenario_.clearWellsAndCalibrationTargets();

  CalibrationTargetCreator targetCreator(scenario_, scenario_.calibrationTargetManager());
  targetCreator.createFromExcel(calibrationTargetsFilename);

  emit signalRefreshChildWidgets();
}

}  // namespace casaWizard

}  // namespace ua
