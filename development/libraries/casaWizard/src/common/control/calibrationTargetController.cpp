#include "calibrationTargetController.h"

#include "model/casaScenario.h"
#include "view/calibrationTargetTable.h"

#include <QCheckBox>
#include <QTableWidgetItem>

namespace casaWizard
{

CalibrationTargetController::CalibrationTargetController(CalibrationTargetTable* calibTable,
                                                         CasaScenario& casaScenario,
                                                         QObject* parent) :
  QObject{parent},
  calibrationTable_{calibTable},
  casaScenario_{casaScenario}
{
  connect(calibrationTable_, SIGNAL(checkBoxChanged(int, int)),
          this, SLOT(slotCalibrationTargetCheckBoxStateChanged(int, int)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void CalibrationTargetController::slotRefresh()
{
  const CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
  calibrationTable_->updateTable(calibrationTargetManager.wells());
}

void CalibrationTargetController::slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex)
{
  CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
  calibrationTargetManager.setWellIsActive(state == Qt::Checked, wellIndex);
}

}  // namespace casaWizard
