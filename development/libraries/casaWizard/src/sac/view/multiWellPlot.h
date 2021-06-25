// Widget to display multiple well plots next to eachother
#pragma once

#include <QWidget>

class QHBoxLayout;

namespace casaWizard
{

struct CalibrationTarget;
class CustomCheckbox;
class Legend;

namespace sac
{

class WellPlot;
struct WellTrajectory;

class MultiWellPlot : public QWidget
{
  Q_OBJECT

public:
  explicit MultiWellPlot(QWidget* parent = nullptr);

  void updatePlots(const QVector<QVector<CalibrationTarget> >& targets,
                   const QStringList& units,
                   const QVector<QVector<WellTrajectory> >& allTrajectories,
                   const QVector<bool>& activePlots,
                   const QMap<QString, double>& surfaceLines,
                   const bool fitRangeToData);
  void setExpanded(const bool isExpanded, const int plotID);

signals:
  void isExpandedChanged(int state, int iPlot);

private:
  QVector<WellPlot*> plots_;
  QVector<bool> expandedPlots_;
  QVector<CustomCheckbox*> isExpandedCheckboxes_;
  QWidget* plotSelection_;
  Legend* legend_;
  void clearState();
  void initializePlotSelection(const int nProperties, const QVector<QVector<WellTrajectory> >& allTrajectories);
  QHBoxLayout* obtainPlotSelectionAndLegendLayout();
  void setTotalLayout(QHBoxLayout* plotSelectionAndLegendLayout);
  void drawPlots(const QVector<QVector<CalibrationTarget> >& targets,
                       const QStringList& units,
                       const QVector<QVector<WellTrajectory>>& allTrajectories,
                       const QVector<bool>& activePlots, const QMap<QString, double>& surfaceLines,
                       const bool fitRangeToData, const int nProperties);
  void clearPlots();
  void clearCheckBoxes();
  void clearLegend();
};

} // namespace sac

} // namespace casaWizard
