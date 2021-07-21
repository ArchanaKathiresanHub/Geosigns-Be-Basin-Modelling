#include "multiWellPlot.h"

#include "model/calibrationTarget.h"
#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

#include "view/plot/wellPlot.h"
#include "view/components/customcheckbox.h"
#include "view/plot/legend.h"

#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QFrame>

namespace casaWizard
{

namespace sac
{

MultiWellPlot::MultiWellPlot(QWidget* parent) :
  QWidget(parent),
  plots_{},
  expandedPlots_{},
  isExpandedCheckboxes_{},
  plotSelection_{new QWidget(this)},
  legend_{},
  showSurfaceLines_{new CustomCheckbox("Show surface lines", this)},
  fitRangeToData_{new CustomCheckbox("Fit range to well data", this)}
{  
  setLayout(new QGridLayout());

  connect(showSurfaceLines_, SIGNAL(stateChanged(int)), this, SLOT(slotShowSurfaceLinesChanged(int)));
  connect(fitRangeToData_, SIGNAL(stateChanged(int)), this, SLOT(slotFitRangeToDataChanged(int)));
}

void MultiWellPlot::slotShowSurfaceLinesChanged(int state)
{
  emit showSurfaceLinesChanged(state == Qt::CheckState::Checked);
}

void MultiWellPlot::slotFitRangeToDataChanged(int state)
{
  emit fitRangeToDataChanged(state == Qt::CheckState::Checked);
}

void MultiWellPlot::setActivePropertyUserNames(const QStringList& activePropertyUserNames)
{
  activePropertyUserNames_ = activePropertyUserNames;

  const int nProperties = activePropertyUserNames.size();

  // Reset the selection if the size has changed, default of 4 plots maximum
  if (expandedPlots_.size() != nProperties)
  {
    expandedPlots_ = QVector<bool>(nProperties, true);
    for (int i = 4; i< nProperties; ++i)
    {
      expandedPlots_[i] = false;
    }
  }

  createPlotSelectionHeader();
}

void MultiWellPlot::createPlotSelectionHeader()
{
  QSignalBlocker blocker(this);

  delete plotSelection_;
  isExpandedCheckboxes_.clear();

  plotSelection_ = new QWidget(this);
  QGridLayout* plotSelectionLayout = new QGridLayout(plotSelection_);
  plotSelectionLayout->addWidget(new QLabel("Plot selection: ", plotSelection_), 0, 0);

  const int maxPerRow = 5;
  int row = 0;
  int col = 1;
  for (int i = 0; i < activePropertyUserNames_.size(); i++)
  {
    CustomCheckbox* isExpandedCheckbox = new CustomCheckbox(activePropertyUserNames_[i]);
    isExpandedCheckbox->setChecked(expandedPlots_[i]);
    connect(isExpandedCheckbox, &CustomCheckbox::stateChanged, [=](int state){emit isExpandedChanged(state, i);});

    plotSelectionLayout->addWidget(isExpandedCheckbox, row, col);
    if (col == maxPerRow)
    {
      col = 0;
      row++;
    }
    col++;

    isExpandedCheckboxes_.push_back(isExpandedCheckbox);
  }
}

void MultiWellPlot::setShowSurfaceLines(const bool checked)
{
  showSurfaceLines_->setChecked(checked);
}

void MultiWellPlot::setFitRangeToData(const bool checked)
{
  fitRangeToData_->setChecked(checked);
}

void MultiWellPlot::setTotalLayout()
{
  QGridLayout* total = static_cast<QGridLayout*>(layout());  
  total->addLayout(createPlotOptionsAndLegendLayout(), 0, 0, 1, plots_.size());
  total->addWidget(plotSelection_, 1, 0, 1, plots_.size(), Qt::AlignLeft);

  for (int i = 0; i < plots_.size(); i++)
  {
    total->addWidget(plots_[i], 2, i, 1, 1);
    total->setMargin(0);
    total->setSpacing(0);
  }

  total->setRowStretch(0,0);
  total->setRowStretch(1,0);
  total->setRowStretch(2,1);
}

void MultiWellPlot::updatePlots(const QVector<QVector<const CalibrationTarget*>>& targets,
                                const QStringList& units,
                                const QVector<QVector<WellTrajectory>>& allTrajectories,
                                const QVector<bool>& activePlots,
                                const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
  clearState();  
  drawPlots(targets, units, allTrajectories, activePlots, surfaceLines, fitRangeToData);  
  setTotalLayout();
}

void MultiWellPlot::clearState()
{
  clearPlots();  
  clearLegend();  
}

void MultiWellPlot::clearPlots()
{
  for (WellPlot* plot : plots_)
  {
    delete plot;
  }
  plots_.clear();
}

void MultiWellPlot::clearLegend()
{
  if (legend_)
  {
    delete legend_;
    legend_ = nullptr;
  }
}

void MultiWellPlot::drawPlots(const QVector<QVector<const CalibrationTarget*>>& targets,
                                    const QStringList& units,
                                    const QVector<QVector<WellTrajectory>>& allTrajectories,
                                    const QVector<bool>& activePlots,
                                    const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
  const int nTypes = allTrajectories.size();

  std::pair<double, double> zTotalRange;

  for ( int iProperty = 0 ; iProperty<activePropertyUserNames_.size(); ++iProperty)
  {
    if (!expandedPlots_[iProperty])
    {
      continue;
    }
    WellPlot* plot = new WellPlot(this);
    plots_.push_back(plot);

    plot->setFitRangeToWellData(fitRangeToData);
    for (const QString& surfaceLine : surfaceLines.keys())
    {
      plot->drawSurfaceLine(surfaceLine, surfaceLines[surfaceLine]);
    }
    const QString activePropertyName = activePropertyUserNames_[iProperty];
    plot->setXLabel(activePropertyName);

    int iTargetProperty = 0;
    for (const QVector<const CalibrationTarget*>& target : targets)
    {
      if (target[0]->propertyUserName() == activePropertyName)
      {
        QVector<WellTrajectory> plotTrajectories(nTypes, {});
        for (int i = 0; i < nTypes; i++)
        {
          plotTrajectories[i] = allTrajectories[i][iTargetProperty];
        }

        plot->setData(target, plotTrajectories, activePlots);
        for (const QString& surfaceLine : surfaceLines.keys())
        {
          plot->drawSurfaceLine(surfaceLine, surfaceLines[surfaceLine]);
        }
        plot->setXLabel(activePropertyName + " [" + units[iTargetProperty] + "]" );

        std::pair<double, double> zDataRange = plot->zDataRange();
        if (iProperty == 0)
        {
          zTotalRange = zDataRange;
        }
        else
        {
          if (zDataRange.first < zTotalRange.first) zTotalRange.first = zDataRange.first;
          if (zDataRange.second < zTotalRange.second) zTotalRange.second = zDataRange.second;
        }
        break;
      }
      iTargetProperty++;
    }
  }

  for (WellPlot* plot : plots_)
  {
    plot->setZDataRange(zTotalRange);
    plot->updateMinMaxData();
  }
}

QHBoxLayout* MultiWellPlot::createPlotOptionsAndLegendLayout()
{
  QVBoxLayout* plotOptions = new QVBoxLayout();
  plotOptions->addWidget(showSurfaceLines_);
  plotOptions->addWidget(fitRangeToData_);

  QHBoxLayout* plotOptionsAndLegendLayout = new QHBoxLayout();
  plotOptionsAndLegendLayout->addLayout(plotOptions, 0);
  if (plots_.size() > 0)
  {    
    for ( WellPlot* wellPlot : plots_ )
    {
      if (wellPlot->containsData()) // Some plots might not have any data as the property is not calibrated for this well
      {
        legend_ = new Legend(wellPlot->plotSettings(), wellPlot->plotDataForLegend(), wellPlot->legend(), this);
        plotOptionsAndLegendLayout->addWidget(legend_);
        plotOptionsAndLegendLayout->setStretch(1,1);
        break;
      }
    }
  }
  else
  {
    plotOptionsAndLegendLayout->addSpacerItem(new QSpacerItem(0,0));
    plotOptionsAndLegendLayout->setStretch(1,1);
  }
  plotOptionsAndLegendLayout->setStretch(0,0);

  return plotOptionsAndLegendLayout;
}

void MultiWellPlot::setExpanded(const bool isExpanded, const int plotID)
{
  expandedPlots_[plotID] = isExpanded;
}

} // namespace sac

} // namespace casaWizard
