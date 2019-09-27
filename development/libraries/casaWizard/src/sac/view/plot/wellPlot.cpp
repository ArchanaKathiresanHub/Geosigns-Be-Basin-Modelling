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
  completeLegend_{}
{
  QVector<QString> legend(5, "");
  legend[0] = "Measurement";
  legend[1 + TrajectoryType::Base3D] = "Base 3d case";
  legend[1 + TrajectoryType::Base1D] = "Base 1d case";
  legend[1 + TrajectoryType::Optimized1D] = "Optimized 1d case";
  legend[1 + TrajectoryType::Optimized3D] = "Optimized 3d case";
  completeLegend_ = legend.toList();
  setLegend(completeLegend_);
  setYLabel("Depth [m]");

  setFixedWidth(300);
}

void WellPlot::setData(const QVector<CalibrationTarget>& targets,
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

  const QString property = targets[0].property();

  QVector<QVector<double>> xCoordsHorizontalErrorBars;
  QVector<double> yCoordsHorizontalErrorBars;

  for (const CalibrationTarget& target : targets)
  {
    if (target.property() == property)
    {
      const double depth = -target.z();
      const double value = target.value();
      z.push_back(depth);
      values.push_back(value);

      xCoordsHorizontalErrorBars.push_back({value - target.standardDeviation()*2, value + target.standardDeviation()*2});
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

} // namespace sac

} // namespace casaWizard
