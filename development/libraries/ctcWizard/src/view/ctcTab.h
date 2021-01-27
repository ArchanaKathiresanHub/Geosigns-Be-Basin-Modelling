#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace ctcWizard
{

class LithosphereParameterTable;
class RiftingHistoryTable;

class CTCtab : public QWidget
{
  Q_OBJECT

public:
  explicit CTCtab(QWidget* parent = 0);

  LithosphereParameterTable* lithosphereParameterTable() const;
  RiftingHistoryTable* riftingHistoryTable() const;
  QLineEdit* lineEditProject3D() const;
  QLineEdit* lineEditNumProc() const;
  const QPushButton* pushSelectProject3D() const;
  const QPushButton* pushButtonCTCrunCTC() const;
  const QPushButton* pushButtonRunFastCauldron() const;
  const QComboBox* fastcldrnRunModeComboBox() const;
  const QPushButton* pushButtonCTCoutputMaps() const;
  const QPushButton* pushButtonExportCTCoutputMaps() const;

private:
  QPushButton* pushSelectProject3D_;
  QLineEdit* lineEditProject3D_;
  QLineEdit* lineEditNumProc_;
  QComboBox* fastcldrnRunModeComboBox_;
  QPushButton* pushButtonRunFastCauldron_;

  LithosphereParameterTable* lithosphereParameterTable_;
  RiftingHistoryTable* riftingHistoryTable_;

  QPushButton* pushButtonCTCrunCTC_;
  QPushButton* pushButtonCTCoutputMaps_;
  QPushButton* pushButtonExportCTCoutputMaps_;
};

} // namespace ctcWizard
