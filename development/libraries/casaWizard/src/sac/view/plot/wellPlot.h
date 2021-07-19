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

  void setData(const QVector<const CalibrationTarget*>& targets,
               const QVector<WellTrajectory>& trajectories,
               const QVector<bool> activePlots);

  void drawSurfaceLine(const QString& surfaceName, const double surfaceDepth);

  void updateMinMaxData() override;

  void setFitRangeToWellData(bool fitRangeToWellData);

  std::pair<double, double> zDataRange() const;
  void setZDataRange(const std::pair<double, double>& zDataRange);

private:
  QStringList completeLegend_;
  bool fitRangeToWellData_;
  std::pair<double, double> valueDataRange_;
  std::pair<double, double> zDataRange_;

};

} // namespace sac

} // namespace casaWizard
