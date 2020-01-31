// Tab to define the design of experiments and influential parameters
#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class DoeOption;
class DoeOptionTable;
class InfluentialParameterTable;
class ManualDesignPointTable;

class DoeTab : public QWidget
{
  Q_OBJECT

public:
  explicit DoeTab(QWidget* parent = 0);

  InfluentialParameterTable* influentialParameterTable() const;
  ManualDesignPointTable* manualDesignPointTable() const;

  QLineEdit* lineEditProject3D() const;
  const QPushButton* pushSelectProject3D() const;
  QComboBox* comboBoxApplication() const;
  QComboBox* comboBoxCluster() const;
  QSpinBox* spinBoxCPUs() const;
  const QPushButton* pushButtonDoeRunCASA() const;
  const QPushButton* pushButtonRunAddedCases() const;
  QTableWidget* doeOptionTable() const;
  void updateDoeOptionTable(const QVector<DoeOption*>& doeOptions, const QVector<bool>& isDoeOptionsSelected);
  int columnIndexCheckBoxDoeOptionTable() const;
  int columnIndexNDesignPointsDoeOptionTable() const;

private:
  QPushButton* pushSelectProject3D_;
  QLineEdit* lineEditProject3D_;

  DoeOptionTable* doeOptionTable_;
  InfluentialParameterTable* influentialParameterTable_;
  ManualDesignPointTable* manualDesignPointTable_;

  QComboBox* comboBoxApplication_;
  QComboBox* comboBoxCluster_;
  QSpinBox* spinBoxCPUs_;

  QPushButton* pushButtonDoErunCASA_;
  QPushButton* pushButtonRunAddedCases_;
};

} // namespace ua

} // namespace casaWizard
