// Controller to list the calibration targets grouped by wells. Wells can be activated or deactivated.
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class CalibrationTargetTable;
class CasaScenario;

class CalibrationTargetController : public QObject
{
  Q_OBJECT

public:
  CalibrationTargetController(CalibrationTargetTable* calibTable,
                              CasaScenario& casaScenario,
                              QObject* parent);

  void validateWells();
  void selectAllWells();
  void clearWellSelection();

private slots:
  void slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex);

  void slotRefresh();

private:

  CalibrationTargetTable* calibrationTable_;
  CasaScenario& casaScenario_;
};

}  // namespace casaWizard
