//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "correlationWidget.h"
#include "model/influentialParameter.h"
#include "view/plot/correlationPlot.h"
#include "view/plot/histogram.h"
#include <QGridLayout>
#include <QWheelEvent>

namespace casaWizard
{

namespace ua
{

CorrelationWidget::CorrelationWidget(QWidget* parent) :
   QWidget(parent),
   plots_{},
   nInfluentialParameters_{0},
   plotWidth_{500.0},
   plotHeight_{300.0}
{
   setLayout(new QGridLayout());
}

void CorrelationWidget::updateLayoutPlots(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected,
                                          const QVector<QVector<double>>& parameterMatrix, const QVector<double>& rmse)
{
   for (PlotBase* plot : plots_)
   {
      delete plot;
   }

   plots_.clear();

   QGridLayout* layoutPlot = static_cast<QGridLayout*>(layout());

   int row = 0;
   for (int i = 0; i < influentialParameters.size(); ++i)
   {
      if (isInfluentialParameterSelected[i])
      {
         int col = 0;
         for (int j = 0; j < influentialParameters.size(); ++j)
         {
            if (isInfluentialParameterSelected[j])
            {
               if (row == col)
               {
                  Histogram* histogramPlot = new Histogram(this);
                  histogramPlot->setData(parameterMatrix[i]);
                  histogramPlot->update();
                  histogramPlot->setXLabel(influentialParameters[i]->label());
                  histogramPlot->setYLabel("Number of counts");
                  histogramPlot->setFontStyle(FontStyle::small);
                  plots_.push_back(histogramPlot);
                  layoutPlot->addWidget(histogramPlot, row, col);
               }
               else
               {
                  CorrelationPlot* correlationPlot = new CorrelationPlot(this);
                  correlationPlot->setCorrelations(parameterMatrix[i], parameterMatrix[j], rmse);
                  correlationPlot->update();
                  correlationPlot->setXLabel(influentialParameters[i]->label());
                  correlationPlot->setYLabel(influentialParameters[j]->label());
                  correlationPlot->setFontStyle(FontStyle::small);
                  plots_.push_back(correlationPlot);
                  layoutPlot->addWidget(correlationPlot, row, col);
               }
               ++col;
            }
         }
         ++row;
      }
   }

   nInfluentialParameters_ = row;
   setFixedSize(nInfluentialParameters_*plotWidth_, nInfluentialParameters_*plotHeight_);
}

void CorrelationWidget::wheelEvent(QWheelEvent* event)
{
   if (event->modifiers().testFlag(Qt::ControlModifier))
   {
      if (event->delta() > 0)
      {
         plotWidth_ *= 1.1;
         plotHeight_ *= 1.1;
      }
      else if (event->delta() < 0)
      {
         plotWidth_ /= 1.1;
         plotHeight_ /= 1.1;
      }

      setFixedSize(nInfluentialParameters_*plotWidth_, nInfluentialParameters_*plotHeight_);
      update();
   }
   else
   {
      QWidget::wheelEvent(event);
   }
}

} // namespace ua
} // namespace casaWizard
