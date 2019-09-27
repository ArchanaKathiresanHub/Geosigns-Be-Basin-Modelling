#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace casaWizard
{

class CalibrationTargetTable;
class InfluentialParameterTable;
class ObjectiveFunctionTable;
class PredictionTargetTable;

namespace sac
{

class LithofractionTable;

class SACtab : public QWidget
{
  Q_OBJECT

public:
  explicit SACtab(QWidget* parent = 0);

  CalibrationTargetTable* calibrationTargetTable() const;
  LithofractionTable* lithofractionTable() const;
  ObjectiveFunctionTable* objectiveFunctionTable() const;

  QLineEdit* lineEditProject3D() const;
  const QPushButton* pushSelectProject3D() const;
  const QLineEdit* lineEditID() const;
  QComboBox* comboBoxApplication() const;
  QComboBox* comboBoxCluster() const;  
  QLineEdit* lineEditCalibration() const;
  const QPushButton* pushSelectCalibration() const;
  const QPushButton* pushButtonSACrunCASA() const;

private:
  QPushButton* pushSelectProject3D_;
  QLineEdit* lineEditProject3D_;

  CalibrationTargetTable* calibrationTargetTable_;
  LithofractionTable* lithofractionTable_;
  ObjectiveFunctionTable* objectiveFunctionTable_;

  QLineEdit* lineEditCalibration_;
  QPushButton* pushSelectCalibration_;
  QComboBox* comboBoxApplication_;
  QComboBox* comboBoxCluster_;

  QPushButton* pushButtonSACrunCASA_;
};

} // namespace sac

} // namespace casaWizard
