//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "plotOptions.h"

#include "model/trajectoryType.h"

#include "../common/view/components/customcheckbox.h"
#include "../common/view/components/customradiobutton.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace casaWizard
{

namespace sac
{

PlotOptions::PlotOptions(QWidget* parent) :
  QWidget(parent),
  original1d_{new CustomCheckbox("Original 1d", this)},
  optimized1d_{new CustomCheckbox("Optimized 1d", this)},
  original3d_{new CustomCheckbox("Original 3d", this)},
  optimized3d_{new CustomCheckbox("Optimized 3d", this)},
  plotType_{new QButtonGroup(this)},
  table_{new CustomRadioButton("Table", this)},
  depthPlot_{new CustomRadioButton("Depth plot", this)},
  correlationPlot_{new CustomRadioButton("Correlation plot", this)}
{
  depthPlot_->setChecked(true);
  plotType_->addButton(depthPlot_, 0);
  plotType_->addButton(correlationPlot_, 1);
  plotType_->addButton(table_, 2);

  QGridLayout* layout = new QGridLayout();
  layout->addWidget(new QLabel("Plot options", this), 0, 0);
  layout->addWidget(original1d_, 1, 0);
  layout->addWidget(optimized1d_, 1, 1);
  layout->addWidget(original3d_, 2, 0);
  layout->addWidget(optimized3d_, 2, 1);
  layout->addWidget(depthPlot_, 3, 0);
  layout->addWidget(correlationPlot_, 4, 0);
  layout->addWidget(table_, 5, 0);

  setLayout(layout);

  connect(original1d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(original3d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(optimized1d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(optimized3d_, SIGNAL(released()), this, SIGNAL(activeChanged()));

  connect(plotType_, SIGNAL(buttonToggled(int,bool)), this, SLOT(plotTypeButtonToggle(int,bool)));
}

void PlotOptions::plotTypeButtonToggle(int index, bool checked)
{
  if (checked)
  {
    emit plotTypeChange(index);
  }
}

QVector<bool> PlotOptions::activePlots() const
{
  QVector<bool> active(4, false);

  active[TrajectoryType::Original1D] = original1d_->isChecked();
  active[TrajectoryType::Optimized1D] = optimized1d_->isChecked();
  active[TrajectoryType::Original3D] = original3d_->isChecked();
  active[TrajectoryType::Optimized3D] = optimized3d_->isChecked();

  return active;
}

void PlotOptions::setActivePlots(const QVector<bool> activePlots)
{
  original1d_->setChecked(activePlots[TrajectoryType::Original1D]);
  optimized1d_->setChecked(activePlots[TrajectoryType::Optimized1D]);
  original3d_->setChecked(activePlots[TrajectoryType::Original3D]);
  optimized3d_->setChecked(activePlots[TrajectoryType::Optimized3D]);
}


}  // namespace sac

}  // namespace casaWizard
