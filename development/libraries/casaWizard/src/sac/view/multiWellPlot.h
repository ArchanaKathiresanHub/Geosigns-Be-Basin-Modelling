// Widget to display multiple well plots next to eachother
#pragma once

#include <QWidget>

namespace casaWizard
{
struct CalibrationTarget;

namespace sac
{

class WellPlot;
struct WellTrajectory;

class MultiWellPlot : public QWidget
{
  Q_OBJECT

public:
  explicit MultiWellPlot(QWidget* parent = 0);
  void updatePlots(const QVector<QVector<CalibrationTarget> > targets,
                   const QStringList& properties,
                   const QVector<QVector<WellTrajectory> > allTrajectories,
                   const QVector<bool>& activePlots);

private:
  QVector<WellPlot*> plots_;
};

} // namespace sac

} // namespace casaWizard
