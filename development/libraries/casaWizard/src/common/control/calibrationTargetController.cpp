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
  calibrationTable_{calibTable},
  casaScenario_{casaScenario}
{
  connect(calibrationTable_, SIGNAL(checkBoxChanged(int, int)),
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

void CalibrationTargetController::validateWells()
{  
  calibrationTargetManager().disableInvalidWells(casaScenario_.project3dPath().toStdString(), casaScenario_.projectReader().getDepthGridName(0).toStdString());
}

void CalibrationTargetController::slotSelectAllWells()
{
  calibrationTable_->selectAllWells();
}

void CalibrationTargetController::slotClearWellSelection()
{
  calibrationTable_->clearWellSelection();
}

void CalibrationTargetController::slotRefresh()
{  
  calibrationTable_->updateTable(calibrationTargetManager().wells(), getPropertyNamesPerWell());
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
