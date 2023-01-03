// This tab provides the time to depth (t2z) calibration input and calculation button
#pragma once

#include <QWidget>

class QSpinBox;
class QPushButton;
class QComboBox;

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class T2Ztab : public QWidget
{
  Q_OBJECT

public:
  explicit T2Ztab(QWidget* parent = 0);

  const QPushButton* pushButtonSACrunT2Z() const;
  const QPushButton* exportT2ZScenario() const;
  const QPushButton* exportT2ZMapsToZycor() const;
  QComboBox* comboBoxProjectSelection() const;
  QComboBox* comboBoxReferenceSurface() const;
  QSpinBox* spinBoxSubSampling() const;
  QSpinBox* spinBoxNumberOfCPUs() const;
  QComboBox* comboBoxClusterSelection() const;

  void addProjectSelectionOptions(const QStringList& options);
  void setReferenceSurfaces(const QStringList& surfaces);
  bool noProjectAvailable() const;

private:
  QPushButton* pushButtonSACrunT2Z_;
  QComboBox* comboBoxReferenceSurface_;
  QComboBox* comboBoxProjectSelection_;
  QSpinBox* spinBoxSubSampling_;
  QSpinBox* spinBoxNumberOfCPUs_;
  QComboBox* comboBoxClusterSelection_;
  QPushButton* exportT2ZScenario_;
  QPushButton* exportT2ZMapsToZycor_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
