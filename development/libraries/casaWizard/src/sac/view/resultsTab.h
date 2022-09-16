// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>

class CustomTitle;

class QButtonGroup;
class QLabel;
class QListWidget;
class QPushButton;
class QStackedLayout;
class QTableWidget;
class QTableWidgetItem;

namespace casaWizard
{

class ProjectReader;
class ColorMap;
struct CalibrationTarget;
struct Well;

namespace sac
{

class MultiWellPlot;
class PlotOptions;
class WellBirdsView;
class WellCorrelationPlot;
class WellCorrelationPlotLayout;
struct Lithofraction;
struct OptimizedLithofraction;
struct WellTrajectory;

class ResultsTab : public QWidget
{
  Q_OBJECT

public:
  explicit ResultsTab(QWidget* parent = 0);

  QListWidget* wellsList() const;  
  PlotOptions* plotOptions() const;
  WellBirdsView* wellBirdsView() const;
  WellCorrelationPlot* wellCorrelationPlot() const;
  WellCorrelationPlotLayout* wellCorrelationPlotLayout() const;
  MultiWellPlot* multiWellPlot() const;
  QButtonGroup* wellPrepOrSAC() const;

  void clearWellCorrelationPlot();

  void updateWellList(const QVector<const Well*> wells);
  void updateWellPlot(const QVector<QVector<const CalibrationTarget*> > targets,
                      const QStringList units,
                      const QVector<QVector<WellTrajectory>> allTrajectories,
                      const QVector<bool>& activePlots,
                      const QMap<QString, double>& surfaceLines, const bool fitRangeToData);
  void updateCorrelationPlot(const QVector<QVector<double> >& measuredValueTrajectories,
                         const QVector<QVector<double> >& simulatedValueTrajectories,
                         const QString activeProperty,
                         const QVector<bool> activePlots, const double minValue, const double maxValue, const QVector<int>& wellIndices);
  void updateOptimizedLithoTable(const QStringList& layerNameList,
                                 const QVector<QStringList>& lithoNamesVector,
                                 const QVector<QVector<double> >& originalValuesVector,
                                 const QVector<QVector<double> >& optimizedValuesVector);
  void updateBirdsView(const QVector<const Well*> wells);
  void updateSelectedWells();
  void updateActivePropertyUserNames(const QStringList& activePropertyUserNames);

  void setPlotType(const int currentIndex);

  void setRangeBirdsView(const double xMin, const double xMax, const double yMin, const double yMax);

private:
  void addLithofractionRow(int &row, QVector<QTableWidgetItem*> items, const double diff);
  void setVisibleLithofractionColumn(const bool visible);
  QTableWidgetItem* createHeaderItem(const QString& name, int align);

  QButtonGroup* wellPrepOrSAC_;
  QPushButton* selectAll_;
  QPushButton* selectClear_;
  QListWidget* wellsList_;
  QTableWidget* optimizedLithoTable_;
  MultiWellPlot* multiWellPlot_;
  PlotOptions* plotOptions_;
  QStackedLayout* layoutStackedPlots_;
  CustomTitle* tableLable_;
  ColorMap* colorMap_;
  WellBirdsView* wellBirdsView_;
  WellCorrelationPlotLayout* wellCorrelationPlotLayout_;
};

} // namespace sac

} // namespace casaWizard
