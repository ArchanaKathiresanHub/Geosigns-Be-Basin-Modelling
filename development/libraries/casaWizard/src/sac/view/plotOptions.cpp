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
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

namespace casaWizard
{

namespace sac
{

PlotOptions::PlotOptions(QWidget* parent) :
  QWidget(parent),
  base1d_{new CustomCheckbox("Base 1d", this)},
  optimized1d_{new CustomCheckbox("Optimized 1d", this)},
  base3d_{new CustomCheckbox("Base 3d", this)},
  optimized3d_{new CustomCheckbox("Optimized 3d", this)},
  properties_{new QComboBox(this)},
  plotType_{new QButtonGroup(this)},
  table_{new CustomRadioButton("Table", this)},
  linePlot_{new CustomRadioButton("Line plot", this)},
  scatterPlot_{new CustomRadioButton("Scatter plot", this)}
{
  table_->setChecked(true);
  plotType_->addButton(table_, 0);
  plotType_->addButton(linePlot_, 1);
  plotType_->addButton(scatterPlot_, 2);

  QGridLayout* layout = new QGridLayout();
  layout->addWidget(new QLabel("Plot options", this), 0, 0);
  layout->addWidget(base1d_, 1, 0);
  layout->addWidget(optimized1d_, 1, 1);
  layout->addWidget(base3d_, 2, 0);
  layout->addWidget(optimized3d_, 2, 1);
  layout->addWidget(table_, 3, 0);
  layout->addWidget(linePlot_, 4, 0);
  layout->addWidget(scatterPlot_, 5, 0);

  properties_->setVisible(false);
  layout->addWidget(properties_, 5, 1, 2, 1);
  setLayout(layout);

  connect(base1d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(base3d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(optimized1d_, SIGNAL(released()), this, SIGNAL(activeChanged()));
  connect(optimized3d_, SIGNAL(released()), this, SIGNAL(activeChanged()));

  connect(plotType_, SIGNAL(buttonToggled(int,bool)), this, SLOT(plotTypeButtonToggle(int,bool)));
  connect(properties_, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(propertyChanged(QString)));
}

void PlotOptions::plotTypeButtonToggle(int index, bool checked)
{
  if (checked)
  {
    emit plotTypeChange(index);
    switch (index)
    {
      case 2:
        properties_->setVisible(true);
        break;
      default:
        properties_->setVisible(false);
    }
  }
}

QVector<bool> PlotOptions::activePlots() const
{
  QVector<bool> active(4, false);

  active[TrajectoryType::Base1D] = base1d_->isChecked();
  active[TrajectoryType::Optimized1D] = optimized1d_->isChecked();
  active[TrajectoryType::Base3D] = base3d_->isChecked();
  active[TrajectoryType::Optimized3D] = optimized3d_->isChecked();

  return active;
}

void PlotOptions::setActivePlots(const QVector<bool> activePlots)
{
  base1d_->setChecked(activePlots[TrajectoryType::Base1D]);
  optimized1d_->setChecked(activePlots[TrajectoryType::Optimized1D]);
  base3d_->setChecked(activePlots[TrajectoryType::Base3D]);
  optimized3d_->setChecked(activePlots[TrajectoryType::Optimized3D]);
}

void PlotOptions::setProperties(const QStringList& properties, const int activeIndex)
{
  properties_->clear();
  properties_->insertItems(0, properties);
  properties_->setCurrentIndex(activeIndex);
}

}  // namespace sac

}  // namespace casaWizard
