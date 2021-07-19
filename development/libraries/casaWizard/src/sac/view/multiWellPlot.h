// Widget to display multiple well plots next to eachother
#pragma once

#include <QWidget>

class QCheckBox;
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

  void updatePlots(const QVector<QVector<const CalibrationTarget*> >& targets,
                   const QStringList& units,
                   const QVector<QVector<WellTrajectory> >& allTrajectories,
                   const QVector<bool>& activePlots,
                   const QMap<QString, double>& surfaceLines,
                   const bool fitRangeToData);
  void setExpanded(const bool isExpanded, const int plotID);

  void setShowSurfaceLines(const bool checked);
  void setFitRangeToData(const bool checked);

  void setActivePropertyUserNames(const QStringList& activePropertyUserNames);

signals:
  void isExpandedChanged(int state, int iPlot);
  void showSurfaceLinesChanged(bool showSurfaceLines);
  void fitRangeToDataChanged(bool fitRangeToData);

private slots:
  void slotShowSurfaceLinesChanged(int state);
  void slotFitRangeToDataChanged(int state);

private:
  QVector<WellPlot*> plots_;
  QStringList activePropertyUserNames_;
  QVector<bool> expandedPlots_;
  QVector<CustomCheckbox*> isExpandedCheckboxes_;
  QWidget* plotSelection_;
  Legend* legend_;
  QCheckBox* showSurfaceLines_;
  QCheckBox* fitRangeToData_;

  void clearState();
  void createPlotSelectionHeader();
  QHBoxLayout* createPlotOptionsAndLegendLayout();
  void setTotalLayout();
  void drawPlots(const QVector<QVector<const CalibrationTarget*>>& targets,
                       const QStringList& units,
                       const QVector<QVector<WellTrajectory>>& allTrajectories,
                       const QVector<bool>& activePlots, const QMap<QString, double>& surfaceLines,
                       const bool fitRangeToData);
  void clearPlots();
  void clearCheckBoxes();
  void clearLegend();
};

} // namespace sac

} // namespace casaWizard
