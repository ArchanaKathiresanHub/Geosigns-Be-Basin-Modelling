//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "wellCorrelationPlotLayout.h"

#include "view/plots/wellCorrelationPlot.h"
#include "view/plots/legend.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

WellCorrelationPlotLayout::WellCorrelationPlotLayout(QWidget *parent) :
   QWidget(parent),
   wellCorrelationPlot_{new WellCorrelationPlot(this)},
   properties_{new QComboBox(this)},
   absoluteErrorRange_{new QDoubleSpinBox(this)},
   relativeErrorRange_{new QDoubleSpinBox(this)}
{
   QGridLayout* layout = new QGridLayout(this);

   QWidget* options = createOptions();

   absoluteErrorRange_->setObjectName("absoluteErrorRange");
   relativeErrorRange_->setObjectName("relativeErrorRange");
   layout->addWidget(options,1,1, Qt::AlignLeft|Qt::AlignTop);
   connect(properties_, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(propertyChanged(QString)));
   connect(absoluteErrorRange_, SIGNAL(valueChanged(double)), this, SLOT(slotAbsoluteErrorChanged(double)));
   connect(relativeErrorRange_, SIGNAL(valueChanged(double)), this, SLOT(slotRelativeErrorChanged(double)));
}

QWidget* WellCorrelationPlotLayout::createOptions()
{
   QWidget* options = new QWidget(this);
   options->setFixedWidth(300);
   options->setFixedHeight(100);
   QGridLayout* optionsLayout = new QGridLayout(options);
   optionsLayout->addWidget(new QLabel("Property"),0,0);
   optionsLayout->addWidget(properties_,0,1);
   optionsLayout->addWidget(new QLabel("Relative error margin (%)"),1,0);
   optionsLayout->addWidget(relativeErrorRange_,1,1);
   relativeErrorRange_->setMinimum(0.0);
   relativeErrorRange_->setMaximum(99.0);
   relativeErrorRange_->setValue(10.0);
   optionsLayout->addWidget(new QLabel("Absolute error margin"),2,0);
   absoluteErrorRange_->setMinimum(0.0);
   absoluteErrorRange_->setMaximum(1e8);
   absoluteErrorRange_->setValue(0.0);
   optionsLayout->addWidget(absoluteErrorRange_,2,1);
   return options;
}

WellCorrelationPlot* WellCorrelationPlotLayout::wellCorrelationPlot() const
{
   return wellCorrelationPlot_;
}

void WellCorrelationPlotLayout::updateCorrelationPlot(const QVector<QVector<double>>& measuredValueTrajectories,
                                                      const QVector<QVector<double>>& simulatedValueTrajectories,
                                                      const QString activeProperty,
                                                      const QVector<bool> activePlots,
                                                      const double minValue,
                                                      const double maxValue,
                                                      const QVector<int>& wellIndices)
{
   if (legend_ != nullptr)
   {
      delete legend_;
      legend_ = nullptr;
   }

   if(activeProperty == "")
   {
      wellCorrelationPlot_->clear();
      return;
   }

   wellCorrelationPlot_->setData(measuredValueTrajectories,
                                 simulatedValueTrajectories,
                                 activeProperty,
                                 activePlots,
                                 minValue,
                                 maxValue,
                                 wellIndices);

   legend_ = new Legend(wellCorrelationPlot_->plotSettings(), wellCorrelationPlot_->plotDataForLegend(), wellCorrelationPlot_->legend(), this);
   legend_->setFixedHeight(20);
   QGridLayout* gridLayout = static_cast<QGridLayout*>(layout());

   gridLayout->addWidget(legend_, 0,0,1,2);
   gridLayout->addWidget(wellCorrelationPlot_,1,0);
   update();
   wellCorrelationPlot_->update();
}

void WellCorrelationPlotLayout::setProperties(const QStringList& properties, const int activeIndex)
{
   properties_->clear();
   properties_->insertItems(0, properties);
   properties_->setCurrentIndex(activeIndex);
}

void WellCorrelationPlotLayout::slotAbsoluteErrorChanged(double absoluteError)
{
   wellCorrelationPlot_->setAbsoluteErrorMargin(absoluteError);
   wellCorrelationPlot_->update();
}

void WellCorrelationPlotLayout::slotRelativeErrorChanged(double relativeError)
{
   wellCorrelationPlot_->setRelativeErrorMargin(relativeError/100);
   wellCorrelationPlot_->update();
}

} // namespace sac

} // namespace casaWizard
