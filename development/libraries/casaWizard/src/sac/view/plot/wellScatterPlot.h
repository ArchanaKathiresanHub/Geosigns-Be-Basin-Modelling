// Scatter plot of the simulated versus the measured data
#pragma once

#include "model/trajectoryType.h"
#include "view/plot/plot.h"

namespace casaWizard
{

struct CalibrationTarget;

namespace sac
{

struct WellTrajectory;

class WellScatterPlot : public Plot
{
  Q_OBJECT

public:
  explicit WellScatterPlot(QWidget* parent = 0);

  void setData(const QVector<QVector<CalibrationTarget> >& targets,
               const QVector<QVector<WellTrajectory>>& allTrajectories,
               const QString property,
               const QVector<bool> activePlots);
  void clear();

signals:
  void selectedWell(int wellIndex);

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void selectedPoint(int plot, int point);

private:
  QStringList completeLegend_;
  QVector<int> wellIndices_;
};

}  // namespace sac

}  // namespace casaWizard
