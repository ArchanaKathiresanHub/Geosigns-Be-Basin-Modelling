// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;
class QStackedLayout;
class QTableWidget;

namespace casaWizard
{

class ProjectReader;
struct CalibrationTarget;
struct Well;

namespace sac
{

class MultiWellPlot;
class PlotOptions;
class WellBirdsView;
class WellScatterPlot;

struct Lithofraction;
struct OptimizedLithofraction;
struct WellTrajectory;

class ResultsTab : public QWidget
{
  Q_OBJECT

public:
  explicit ResultsTab(QWidget* parent = 0);

  QListWidget* wellsList() const;
  QPushButton* buttonSaveOptimized() const;
  QPushButton* buttonRunOptimized() const;
  QPushButton* buttonBaseCase() const;
  PlotOptions* plotOptions() const;
  WellBirdsView* wellBirdsView() const;
  WellScatterPlot* wellScatterPlot() const;

  void updateWellList(const QVector<Well>& wells);
  void updateWellPlot(const QVector<QVector<CalibrationTarget>> targets,
                      const QStringList properties,
                      const QVector<QVector<WellTrajectory>> allTrajectories,
                      const QVector<bool>& activePlots);
  void updateScatterPlot(const QVector<QVector<CalibrationTarget>> targets,
                         const QStringList properties,
                         const QVector<QVector<WellTrajectory>> allTrajectories,
                         const QVector<bool> activePlots,
                         const QString activeProperty);
  void updateOptimizedLithoTable(const QVector<OptimizedLithofraction>& optimizedLithofractions,
                                 const QVector<Lithofraction>& lithofractions,
                                 const ProjectReader& projectReader);
  void updateBirdsView(const QVector<const Well*> wells);
  void updateActiveWells(const QVector<int> activeWells);

  void setPlotType(const int currentIndex);

private:
  void setVisibleLithofractionColumn(const bool visible);

  QListWidget* wellsList_;
  QTableWidget* optimizedLithoTable_;
  MultiWellPlot* multiWellPlot_;
  WellScatterPlot* wellScatterPlot_;
  QPushButton* buttonSaveOptimized_;
  QPushButton* buttonRunOptimized_;
  QPushButton* buttonBaseCase_;
  PlotOptions* plotOptions_;
  QLabel* tableLable_;
  QStackedLayout* layoutStackedPlots_;
  WellBirdsView* wellBirdsView_;
};

} // namespace sac

} // namespace casaWizard
