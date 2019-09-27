#include "t2zTab.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace casaWizard
{

namespace sac
{

T2Ztab::T2Ztab(QWidget* parent) :
  QWidget(parent),
  pushButtonSACrunT2Z_{new QPushButton("Run CASA", this)},
  spinBoxReferenceSurface_{new QSpinBox(this)},
  spinBoxLastSurface_{new QSpinBox(this)}
{
  QGridLayout* layout = new QGridLayout();
  layout->addWidget(new QLabel("Reference surface", this), 0, 0);
  layout->addWidget(spinBoxReferenceSurface_, 0, 1);
  layout->addWidget(new QLabel("Last surface", this), 1, 0);
  layout->addWidget(spinBoxLastSurface_, 1, 1);
  layout->addWidget(pushButtonSACrunT2Z_, 2, 1);

  layout->addWidget(new QWidget(this), 3, 2);
  layout->setRowStretch(3,1);
  layout->setColumnStretch(2,1);

  setLayout(layout);
}

const QPushButton* T2Ztab::pushButtonSACrunT2Z() const
{
  return pushButtonSACrunT2Z_;
}

QSpinBox* T2Ztab::spinBoxReferenceSurface() const
{
  return spinBoxReferenceSurface_;
}

QSpinBox* T2Ztab::spinBoxLastSurface() const
{
  return spinBoxLastSurface_;
}

} // namespace sac

} // namespace casaWizard
