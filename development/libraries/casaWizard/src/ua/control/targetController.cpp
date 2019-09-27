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
  connect(targetTab_->pushSelectCalibration(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectCalibrationClicked()));
  connect(targetTab_->lineEditCalibration(),   SIGNAL(textChanged(const QString&)), this, SLOT(slotLineEditCalibrationTextChanged(const QString&)));

  connect(this, SIGNAL(signalRefresh()), calibrationTargetController_, SLOT(slotRefresh()));
  connect(this, SIGNAL(signalRefresh()), objectiveFunctionController_, SLOT(slotRefresh()));
  connect(this, SIGNAL(signalRefresh()), depthTargetController_,       SLOT(slotRefresh()));
  connect(this, SIGNAL(signalRefresh()), surfaceTargetController_,     SLOT(slotRefresh()));
}

void TargetController::slotRefresh(int tabID)
{
  if (tabID == static_cast<int>(TabID::Targets))
  {
    return;
  }

  QSignalBlocker blockCalibration{targetTab_->lineEditCalibration()};
  targetTab_->lineEditCalibration()->setText("");
  emit signalRefresh();
}

void TargetController::slotEnableDisableDependentWorkflowTabs(int tabID, bool hasLogMessage)
{
  if (tabID != static_cast<int>(TabID::Targets) || targetTab_->isEnabled())
  {
    return;
  }

  if (hasLogMessage)
  {
    Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
  }
}


void TargetController::slotPushSelectCalibrationClicked()
{
  QString fileName = QFileDialog::getOpenFileName(targetTab_,
                                                  "Select calibration targets",
                                                  "",
                                                  "Spreadsheet (*.xlsx)");
  targetTab_->lineEditCalibration()->setText(fileName);
  emit signalRefresh();
}

void TargetController::slotLineEditCalibrationTextChanged(const QString& calibrationTargetsFilename)
{
  scenario_.clearWellsAndCalibrationTargets();

  calibrationTargetCreator::createFromExcel(scenario_, calibrationTargetsFilename);
  emit signalRefresh();
}

void TargetController::slotEnableTabAndUpdateDependentWorkflowTabs()
{
  targetTab_->setEnabled(true);
}

void TargetController::slotDisableTabAndUpdateDependentWorkflowTabs()
{
  targetTab_->setEnabled(false);
}

}  // namespace casaWizard

}  // namespace ua
