//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellCorrelationPlot.h"

#include "model/calibrationTarget.h"
#include "model/functions/interpolateVector.h"
#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

#include <QPainter>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

WellCorrelationPlot::WellCorrelationPlot(QWidget* parent) :
  Plot(parent),
  completeLegend_{},
  wellIndices_{},
  minValue_{0.0},
  maxValue_{1.0}
{
  QVector<QString> legend(4, "");
  legend[TrajectoryType::Original1D] = "Original 1d case";
  legend[TrajectoryType::Optimized1D] = "Optimized 1d case";
  legend[TrajectoryType::Original3D] = "Original 3d case";
  legend[TrajectoryType::Optimized3D] = "Optimized 3d case";
  completeLegend_ = legend.toList();
  setLegend(completeLegend_);
  setXLabel("Measurement");
  setYLabel("Simulation");

  connect(this, SIGNAL(pointSelectEvent(int,int)), this, SLOT(selectedPoint(int,int)));
}

void WellCorrelationPlot::setData(const QVector<QVector<CalibrationTarget>>& targets,
                              const QVector<QVector<WellTrajectory>>& allTrajectories,
                              const QString property,
                              const QVector<bool> activePlots)
{
  assert(allTrajectories.size() == 4);
  wellIndices_.clear();
  clearData();
  QStringList activeLegend;
  const int nTrajectories = targets.size();

  minValue_ = 0.0;
  maxValue_ = 1.0;
  bool first = true;

  for (int j = 0; j < allTrajectories.size(); ++j)
  {
    const bool wellIndicesSet = wellIndices_.size()>0;
    if (!activePlots[j])
    {
      continue;
    }
    activeLegend.append(completeLegend_[j]);

    QVector<double> allValuesMeasured;
    QVector<double> allValuesSimulated;
    for (int i = 0; i < nTrajectories; ++i)
    {
      if (allTrajectories[j][i].property() != property ||
          allTrajectories[j][i].depth().empty())
      {
        continue;
      }

      QVector<double> depthMeasured;
      QVector<double> valueMeasured;
      for (const CalibrationTarget& target : targets[i])
      {
        assert(target.property() == property);
        depthMeasured.push_back(target.z());
        valueMeasured.push_back(target.value());
      }

      QVector<double> valueSimulated = functions::interpolateVector(allTrajectories[j][i].depth(), allTrajectories[j][i].value(), depthMeasured);

      for (int k = 0; k < valueSimulated.size(); k++)
      {
        allValuesMeasured.append(valueMeasured[k]);
        allValuesSimulated.append(valueSimulated[k]);
        if (!wellIndicesSet)
        {
          wellIndices_.append(allTrajectories[j][i].wellIndex());
        }

        if (first)
        {
            minValue_ = valueMeasured[k];
            maxValue_ = valueMeasured[k];
            first = false;
        }

        if (minValue_ > valueMeasured[k])  minValue_ = valueMeasured[k];
        if (minValue_ > valueSimulated[k]) minValue_ = valueSimulated[k];
        if (maxValue_ < valueMeasured[k])  maxValue_ = valueMeasured[k];
        if (maxValue_ < valueSimulated[k]) maxValue_ = valueSimulated[k];
      }
    }
    addXYscatter(allValuesMeasured, allValuesSimulated, j);
  }

  setXLabel("Measured " + property);
  setYLabel("Simulated " + property);
  setLegend(activeLegend);
  update();
}

void WellCorrelationPlot::clear()
{
  clearData();
  update();
}

void WellCorrelationPlot::updateMinMaxData()
{
  setMinMaxValues(minValue_, maxValue_, minValue_, maxValue_);
}

void WellCorrelationPlot::resizeEvent(QResizeEvent* event)
{
  const QSize currentSize = size();
  const int min = std::min(currentSize.width(), currentSize.height());
  resize(min, min);
  Plot::resizeEvent(event);
}

void WellCorrelationPlot::selectedPoint(int plot, int point)
{
  if (point >= wellIndices_.size())
  {
    return;
  }
  emit selectedWell(wellIndices_[point]);
}

void WellCorrelationPlot::paintEvent(QPaintEvent* event)
{
  Plot::paintEvent(event);

  QPainter painter(this);
  const double minV = yAxisMinValue();
  const double maxV = yAxisMaxValue();

  painter.setPen(Qt::black);
  painter.setPen(Qt::DashDotLine);
  painter.drawLine(valToPoint(minV, minV), valToPoint(maxV, maxV));
}

}  // namespace sac

}  // namespace casaWizard
