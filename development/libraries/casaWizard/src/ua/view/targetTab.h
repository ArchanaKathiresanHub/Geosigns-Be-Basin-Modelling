// Tab in the UA Thermal wizard for creating all targets
#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;

namespace casaWizard
{

class CalibrationTargetTable;
class ObjectiveFunctionTable;

namespace ua
{

class PredictionTargetTable;

class TargetTab : public QWidget
{
  Q_OBJECT

public:
  explicit TargetTab(QWidget* parent = 0);

  CalibrationTargetTable* calibrationTargetTable() const;
  ObjectiveFunctionTable* objectiveFunctionTable() const;
  PredictionTargetTable* surfaceTargetTable() const;
  QLineEdit* lineEditCalibration() const;
  const QPushButton* pushSelectCalibration() const;
  const QPushButton* pushSelectAllTemperatures() const;
  const QPushButton* pushSelectAllVRe() const;
  const QPushButton* pushSelectAllTargetTemperatures() const;
  const QPushButton* pushSelectAllTargetVRe() const;
  const QPushButton* pushSelectAllTargetTimeSeries() const;

private:
  CalibrationTargetTable* calibrationTargetTable_;
  ObjectiveFunctionTable* objectiveFunctionTable_;
  PredictionTargetTable* predictionTargetTable_;

  QLineEdit* lineEditCalibration_;
  QPushButton* pushSelectCalibration_;
  QPushButton* pushSelectAllTemperatures_;
  QPushButton* pushSelectAllVRe_;
  QPushButton* pushSelectAllTargetTemperatures_;
  QPushButton* pushSelectAllTargetVRe_;
  QPushButton* pushSelectAllTargetTimeSeries_;
};

} // namespace ua

} // namespace casaWizard
