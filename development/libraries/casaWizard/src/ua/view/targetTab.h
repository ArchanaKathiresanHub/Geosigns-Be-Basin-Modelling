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

class DepthTargetTable;
class SurfaceTargetTable;

class TargetTab : public QWidget
{
  Q_OBJECT

public:
  explicit TargetTab(QWidget* parent = 0);

  CalibrationTargetTable* calibrationTargetTable() const;
  ObjectiveFunctionTable* objectiveFunctionTable() const;
  DepthTargetTable* depthTargetTable() const;
  SurfaceTargetTable* surfaceTargetTable() const;
  QLineEdit* lineEditCalibration() const;
  const QPushButton* pushSelectCalibration() const;

private:
  DepthTargetTable* depthTargetTable_;
  CalibrationTargetTable* calibrationTargetTable_;
  ObjectiveFunctionTable* objectiveFunctionTable_;
  SurfaceTargetTable* surfaceTargetTable_;

  QLineEdit* lineEditCalibration_;
  QPushButton* pushSelectCalibration_;

};

} // namespace ua

} // namespace casaWizard
