// Controller to list the calibration targets grouped by wells. Wells can be activated or deactivated.
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class CalibrationTargetManager;
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

  virtual CalibrationTargetManager& calibrationTargetManager();
  virtual const CalibrationTargetManager& calibrationTargetManager() const;
  virtual QMap<QString, QSet<int>> getPropertyNamesPerWell() const;

  CasaScenario& casaScenario() const;

public slots:
  void slotSelectAllWells();
  void slotClearWellSelection();

private slots:
  void slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex);
  void slotRefresh();

private:
  CalibrationTargetTable* calibrationTable_;  
  CasaScenario& casaScenario_;
};

}  // namespace casaWizard
