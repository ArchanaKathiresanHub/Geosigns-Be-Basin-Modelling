#include "calibrationTargetController.h"

#include "model/casaScenario.h"
#include "model/input/cmbMapReader.h"
#include "view/calibrationTargetTable.h"

#include <QCheckBox>
#include <QTableWidgetItem>

namespace casaWizard
{

CalibrationTargetController::CalibrationTargetController(CalibrationTargetTable* calibTable,
                                                         CasaScenario& casaScenario,
                                                         QObject* parent) :
  QObject{parent},
  calibrationTargetTable_{calibTable},
  casaScenario_{casaScenario}
{
  connect(calibrationTargetTable_, SIGNAL(checkBoxChanged(int, int)),
          this, SLOT(slotCalibrationTargetCheckBoxStateChanged(int, int)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

CalibrationTargetManager& CalibrationTargetController::calibrationTargetManager()
{
  return casaScenario_.calibrationTargetManager();
}

const CalibrationTargetManager& CalibrationTargetController::calibrationTargetManager() const
{
  return casaScenario_.calibrationTargetManager();
}

QMap<QString, QSet<int>> CalibrationTargetController::getPropertyNamesPerWell() const
{
  return {};
}

void CalibrationTargetController::slotSelectAllWells()
{
  calibrationTargetTable_->selectAllWells();
}

void CalibrationTargetController::slotClearWellSelection()
{
  calibrationTargetTable_->clearWellSelection();
}

void CalibrationTargetController::slotRefresh()
{  
  calibrationTargetTable_->updateTable(calibrationTargetManager().wells(), getPropertyNamesPerWell());
}

CalibrationTargetTable* CalibrationTargetController::calibrationTable() const
{
  return calibrationTargetTable_;
}

CasaScenario& CalibrationTargetController::casaScenario() const
{
  return casaScenario_;
}

void CalibrationTargetController::slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex)
{  
  calibrationTargetManager().setWellIsActive(state == Qt::Checked, wellIndex);
  emit wellSelectionChanged();
}

}  // namespace casaWizard
