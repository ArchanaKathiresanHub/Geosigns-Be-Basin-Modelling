#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class EmphasisButton;

namespace casaWizard
{

class CalibrationTargetTable;
class InfluentialParameterTable;
class ObjectiveFunctionTableSAC;
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
  ObjectiveFunctionTableSAC* objectiveFunctionTable() const;

  QLineEdit* lineEditProject3D() const;
  const QPushButton* pushSelectProject3D() const;  
  QComboBox* comboBoxApplication() const;
  QComboBox* comboBoxCluster() const;    
  const QPushButton* pushSelectCalibration() const;
  const QPushButton* pushSelectAllWells() const;
  const QPushButton* pushClearSelection() const;
  const QPushButton* pushButtonSACrunCASA() const;
  const QPushButton* buttonRunOriginal1D() const;
  const QPushButton* buttonRunOriginal3D() const;

private:
  QPushButton* pushSelectProject3D_;
  QLineEdit* lineEditProject3D_;

  CalibrationTargetTable* calibrationTargetTable_;
  LithofractionTable* lithofractionTable_;
  ObjectiveFunctionTableSAC* objectiveFunctionTable_;

  QPushButton* pushSelectCalibration_;
  QPushButton* pushSelectAllWells_;
  QPushButton* pushClearSelection_;

  QComboBox* comboBoxApplication_;
  QComboBox* comboBoxCluster_;

  EmphasisButton* pushButtonSACrunCASA_;
  QPushButton* buttonRunOriginal1D_;
  QPushButton* buttonRunOriginal3D_;
};

} // namespace sac

} // namespace casaWizard
