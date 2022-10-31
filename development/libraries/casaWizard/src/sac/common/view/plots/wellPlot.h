// Plot to display the well, and optionally, the stored results
#pragma once

#include "view/plots/plot.h"

namespace casaWizard
{

struct CalibrationTarget;

namespace sac
{
   struct WellTrajectory;
}

namespace sac
{

class WellPlot : public Plot
{
  Q_OBJECT

public:
  explicit WellPlot(QWidget* parent = 0);

  void clearData() override;
  void setData(const QVector<const CalibrationTarget*>& targets,
               const QVector<WellTrajectory>& trajectories,
               const QVector<bool> activePlots);

  void drawSurfaceLine(const QString& surfaceName, const double surfaceDepth);

  void updateMinMaxData() override;

  void setFitRangeToWellData(bool fitRangeToWellData);

  std::pair<double, double> zDataRange() const;
  void setZDataRange(const std::pair<double, double>& zDataRange);

  bool containsData() const;

private:
  QStringList completeLegend_;
  bool fitRangeToWellData_;
  std::pair<double, double> valueDataRange_;
  std::pair<double, double> zDataRange_;
  bool containsData_;

};

} // namespace sac

} // namespace casaWizard
