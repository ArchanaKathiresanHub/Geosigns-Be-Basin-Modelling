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
  connect(calibrationTargetTable_, SIGNAL(activePropertyCheckBoxChanged(int, int, QString)),
          this, SLOT(slotCalibrationTargetCheckBoxStateChanged(int, int, QString)));
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
   calibrationTargetTable_->updateTable(calibrationTargetManager().wells(), calibrationTargetManager().getPropertyNamesPerWellForTargetTable());
}

CalibrationTargetTable* CalibrationTargetController::calibrationTable() const
{
   return calibrationTargetTable_;
}

void CalibrationTargetController::refreshAndEmitDataChanged()
{
   slotRefresh();
   emit wellSelectionChanged();
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

void CalibrationTargetController::slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex, QString property)
{
  calibrationTargetManager().setWellActiveProperty(property, state == Qt::Checked, wellIndex);
  emit wellSelectionChanged();
}

}  // namespace casaWizard
