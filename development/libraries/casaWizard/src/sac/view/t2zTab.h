// This tab provides the time to depth (t2z) calibration input and calculation button
#pragma once

#include <QWidget>

class QSpinBox;
class QPushButton;

namespace casaWizard
{

namespace sac
{

class T2Ztab : public QWidget
{
  Q_OBJECT

public:
  explicit T2Ztab(QWidget* parent = 0);

  const QPushButton* pushButtonSACrunT2Z() const;
  QSpinBox* spinBoxReferenceSurface() const;
  QSpinBox* spinBoxLastSurface() const;

private:
  QPushButton* pushButtonSACrunT2Z_;
  QSpinBox* spinBoxReferenceSurface_;
  QSpinBox* spinBoxLastSurface_;
};

} // namespace sac

} // namespace casaWizard
