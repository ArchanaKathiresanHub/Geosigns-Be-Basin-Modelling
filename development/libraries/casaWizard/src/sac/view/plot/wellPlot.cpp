#include "wellPlot.h"

#include "model/calibrationTarget.h"
#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

#include <assert.h>
namespace casaWizard
{

namespace sac
{

namespace
{

QVector<double> reflect(QVector<double> in)
{
  QVector<double> out;
  for (const double d : in)
  {
    out.push_back(-d);
  }
  return out;
}

}

WellPlot::WellPlot(QWidget* parent) :
  Plot(parent),
  completeLegend_{},
  fitRangeToWellData_{false},
  valueDataRange_{},
  zDataRange_{},
  containsData_{false}
{
  QVector<QString> legend(5, "");
  legend[0] = "Measurement";
  legend[1 + TrajectoryType::Original1D] = "Original 1d case";
  legend[1 + TrajectoryType::Optimized1D] = "Optimized 1d case";
  legend[1 + TrajectoryType::Original3D] = "Original 3d case";
  legend[1 + TrajectoryType::Optimized3D] = "Optimized 3d case";
  completeLegend_ = legend.toList();
  setLegend(completeLegend_);
  setYLabel("Depth [m]");
  setInvertYAxis(true);
  setSeparateLegend(true);

  setMouseTracking(true);

  setMinimumWidth(300);
}

void WellPlot::setData(const QVector<const CalibrationTarget*>& targets,
                       const QVector<WellTrajectory>& trajectories,
                       const QVector<bool> activePlots)
{
  assert(activePlots.size() == 4);
  assert(trajectories.size() == 4);
  if( targets.isEmpty())
  {
    return;
  }
  clearData();

  QVector<double> z;
  QVector<double> values;

  const QString property = targets[0]->propertyUserName();

  QVector<QVector<double>> xCoordsHorizontalErrorBars;
  QVector<double> yCoordsHorizontalErrorBars;

  bool first = true;
  for (const CalibrationTarget* target : targets)
  {
    if (target->propertyUserName() == property)
    {
      const double depth = -target->z();
      const double value = target->value();
      z.push_back(depth);
      values.push_back(value);
      containsData_ = true;

      if (first)
      {
        valueDataRange_ = {value, value};
        zDataRange_ = {depth, depth};
        first = false;
      }

      valueDataRange_.first = value < valueDataRange_.first ? value : valueDataRange_.first;
      valueDataRange_.second = value > valueDataRange_.second ? value : valueDataRange_.second;
      zDataRange_.first = depth < zDataRange_.first ? depth : zDataRange_.first;
      zDataRange_.second = depth > zDataRange_.second ? depth : zDataRange_.second;

      xCoordsHorizontalErrorBars.push_back({value - target->standardDeviation()*2, value + target->standardDeviation()*2});
      yCoordsHorizontalErrorBars.push_back(depth);
    }
  }


  addErrorBarHorizontal(xCoordsHorizontalErrorBars, yCoordsHorizontalErrorBars);

  addXYscatter(values, z);
  QStringList newLegend;
  newLegend.append(completeLegend_[0]);

  for (int i = 0; i<trajectories.size(); ++i)
  {
    if (activePlots[i])
    {
      addLine(trajectories[i].value(), reflect(trajectories[i].depth()), i);
      newLegend.append(completeLegend_[1+i]);
    }
  }

  setLegend(newLegend);
}

void WellPlot::drawSurfaceLine(const QString& surfaceName, const double surfaceDepth)
{
  addHorizontalLine(surfaceName, surfaceDepth);
}

void WellPlot::updateMinMaxData()
{
  if (fitRangeToWellData_)
  {
    const double valueMargin = 0.05*(valueDataRange_.second-valueDataRange_.first);
    const double zMargin = 0.05*(zDataRange_.second-zDataRange_.first);
    setMinMaxValues(valueDataRange_.first - valueMargin,
                    valueDataRange_.second + valueMargin,
                    zDataRange_.first - zMargin,
                    zDataRange_.second + zMargin);
  }
  else
  {
    Plot::updateMinMaxData();
  }
}

void WellPlot::setFitRangeToWellData(bool fitRangeToWellData)
{
  fitRangeToWellData_ = fitRangeToWellData;
}

std::pair<double, double> WellPlot::zDataRange() const
{
  return zDataRange_;
}

void WellPlot::setZDataRange(const std::pair<double, double>& zDataRange)
{
  zDataRange_ = zDataRange;
}

bool WellPlot::containsData() const
{
  return containsData_;
}

void WellPlot::clearData()
{
  Plot::clearData();
  containsData_ = false;
}


} // namespace sac

} // namespace casaWizard
