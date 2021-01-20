// Plot to display the well, and optionally, the stored results
#pragma once

#include "view/plot/plot.h"

namespace casaWizard
{

struct CalibrationTarget;

namespace sac
{

struct WellTrajectory;

class WellPlot : public Plot
{
  Q_OBJECT

public:
  explicit WellPlot(QWidget* parent = 0);

  void setData(const QVector<CalibrationTarget>& targets,
               const QVector<WellTrajectory>& trajectories,
               const QVector<bool> activePlots);

private:
  QStringList completeLegend_;
};

} // namespace sac

} // namespace casaWizard