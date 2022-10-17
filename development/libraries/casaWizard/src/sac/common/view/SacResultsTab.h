//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
class QVBoxLayout;

namespace casaWizard
{

class ColorMap;
class CalibrationTarget;
class Well;

namespace sac
{

class WellTrajectory;
class MultiWellPlot;
class PlotOptions;
class WellBirdsView;
class WellCorrelationPlot;
class WellCorrelationPlotLayout;

class SacResultsTab : public QWidget
{
   Q_OBJECT

public:
   explicit SacResultsTab(QWidget* parent = nullptr);

   QListWidget* wellsList() const;
   PlotOptions* plotOptions() const;
   WellBirdsView* wellBirdsView() const;
   WellCorrelationPlot* wellCorrelationPlot() const;
   WellCorrelationPlotLayout* wellCorrelationPlotLayout() const;
   MultiWellPlot* multiWellPlot() const;

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
   void updateBirdsView(const QVector<const Well*> wells);
   void updateSelectedWells();
   void updateActivePropertyUserNames(const QStringList& activePropertyUserNames);

   virtual void setPlotType(const int currentIndex);

   void setRangeBirdsView(const double xMin, const double xMax, const double yMin, const double yMax);
protected:
   QTableWidgetItem* createHeaderItem(const QString& name, int align);
   QTableWidget* optimizedResultsTable() const;
   QVBoxLayout* layoutWellList() const;
   QPushButton* selectAll() const;
   QPushButton* selectClear() const;

private:
   QVBoxLayout* m_layoutWellList;
   QStackedLayout* m_layoutStackedPlots;
   QPushButton* m_selectAll;
   QPushButton* m_selectClear;
   QListWidget* m_wellsList;
   MultiWellPlot* m_multiWellPlot;
   PlotOptions* m_plotOptions;
   CustomTitle* m_tableLabel;
   ColorMap* m_colorMap;
   WellBirdsView* m_wellBirdsView;
   WellCorrelationPlotLayout* m_wellCorrelationPlotLayout;
   QTableWidget* m_optimizedResultsTable;
};

} // namespace sac

} // namespace casaWizard
