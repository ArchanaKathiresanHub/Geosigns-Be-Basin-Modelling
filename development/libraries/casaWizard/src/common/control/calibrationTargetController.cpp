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
  connect(calibrationTable_, SIGNAL(itemChanged(QTableWidgetItem*)),
          this, SLOT(slotCalibrationTargetCheckBoxStateChanged(QTableWidgetItem*)));
}

void CalibrationTargetController::slotRefresh()
{
  const CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
  calibrationTable_->updateTable(calibrationTargetManager.wells());
}

void CalibrationTargetController::slotCalibrationTargetCheckBoxStateChanged(QTableWidgetItem* item)
{
  if (item->column() == calibrationTable_->checkBoxColumn())
  {
    CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
    calibrationTargetManager.setWellIsActive(item->checkState() == Qt::Checked, item->row());
  }
}

}  // namespace casaWizard
