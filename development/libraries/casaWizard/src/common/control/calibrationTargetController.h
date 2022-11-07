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

  virtual CalibrationTargetManager& calibrationTargetManager();
  virtual const CalibrationTargetManager& calibrationTargetManager() const;

  CasaScenario& casaScenario() const;

  CalibrationTargetTable* calibrationTable() const;
  void refreshAndEmitDataChanged();

public slots:
  void slotSelectAllWells();
  void slotClearWellSelection();
  void slotRefresh();

private slots:
  void slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex);
  void slotCalibrationTargetCheckBoxStateChanged(int state, int wellIndex, QString property);
signals:
  void wellSelectionChanged();

private:
  CalibrationTargetTable* calibrationTargetTable_;
  CasaScenario& casaScenario_;
};

}  // namespace casaWizard
