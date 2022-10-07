//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
struct WellTrajectory;
}

namespace sac
{

class WellPlot;

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
   QStringList activePropertyUserNames_;

   QVector<WellPlot*> m_plots;
   QVector<bool> m_expandedPlots;
   QVector<CustomCheckbox*> m_isExpandedCheckboxes;
   QWidget* m_plotSelection;
   Legend* m_legend;
   QCheckBox* m_showSurfaceLines;
   QCheckBox* m_fitRangeToData;

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
