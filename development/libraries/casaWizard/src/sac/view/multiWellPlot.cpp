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
  plotSelection_{new QWidget(this)}
{
  QGridLayout* layout = new QGridLayout();
  setLayout(layout);
}

void MultiWellPlot::setTotalLayout(QHBoxLayout* plotSelectionAndLegendLayout)
{
  QGridLayout* total = static_cast<QGridLayout*>(layout());
  for (int i = 0; i < plots_.size(); i++)
  {
    total->addWidget(plots_[i],1, i, 1,1);
    total->setMargin(0);
    total->setSpacing(0);
  }
  total->addLayout(plotSelectionAndLegendLayout, 0,0, 1, plots_.size());
  total->setRowStretch(1,1);
  total->setRowStretch(0,0);
}

void MultiWellPlot::updatePlots(const QVector<QVector<CalibrationTarget>>& targets,
                                const QStringList& units,
                                const QVector<QVector<WellTrajectory>>& allTrajectories,
                                const QVector<bool>& activePlots,
                                const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
  clearState();

  const int nProperties = targets.size();
  initializePlotSelection(nProperties, allTrajectories);
  drawPlots(targets, units, allTrajectories, activePlots, surfaceLines, fitRangeToData, nProperties);

  QHBoxLayout* plotSelectionAndLegendLayout = obtainPlotSelectionAndLegendLayout();
  setTotalLayout(plotSelectionAndLegendLayout);
}

void MultiWellPlot::clearState()
{
  clearPlots();
  clearCheckBoxes();
  clearLegend();

  delete plotSelection_;
}

void MultiWellPlot::clearPlots()
{
  for (WellPlot* plot : plots_)
  {
    delete plot;
  }
  plots_.clear();
}

void MultiWellPlot::clearCheckBoxes()
{
  for (const auto* checkbox : isExpandedCheckboxes_)
  {
    delete checkbox;
  }
  isExpandedCheckboxes_.clear();
}

void MultiWellPlot::clearLegend()
{
  if (legend_)
  {
    delete legend_;
    legend_ = nullptr;
  }
}

void MultiWellPlot::initializePlotSelection(const int nProperties, const QVector<QVector<WellTrajectory>>& allTrajectories)
{
  if (nProperties != expandedPlots_.size())
  {
    expandedPlots_ = QVector<bool>(nProperties, true);
  }

  plotSelection_ = new QWidget(this);

  QHBoxLayout* plotSelectionLayout = new QHBoxLayout(plotSelection_);
  plotSelectionLayout->addWidget(new QLabel("Plot selection: ", plotSelection_));
  for (int i = 0; i < nProperties; i++)
  {
    QSignalBlocker blocker(this);
    isExpandedCheckboxes_.push_back(new CustomCheckbox(allTrajectories[0][i].propertyUserName()));
    isExpandedCheckboxes_[i]->setChecked(expandedPlots_[i]);
    plotSelectionLayout->addWidget(isExpandedCheckboxes_[i]);
    connect(isExpandedCheckboxes_[i], &CustomCheckbox::stateChanged, [=](int state){emit isExpandedChanged(state, i);});
  }
}

void MultiWellPlot::drawPlots(const QVector<QVector<CalibrationTarget>>& targets,
                                    const QStringList& units,
                                    const QVector<QVector<WellTrajectory>>& allTrajectories,
                                    const QVector<bool>& activePlots,
                                    const QMap<QString, double>& surfaceLines, const bool fitRangeToData,
                                    const int nProperties)
{
  const int nTypes = allTrajectories.size();
  for (int iProperty = 0; iProperty < nProperties; ++iProperty)
  {
    if (!expandedPlots_[iProperty])
    {
      continue;
    }
    WellPlot* plot = new WellPlot(this);
    QVector<WellTrajectory> plotTrajectories(nTypes, {});
    for (int i = 0; i < nTypes; i++)
    {
      plotTrajectories[i] = allTrajectories[i][iProperty];
    }

    plot->setFitRangeToWellData(fitRangeToData);
    plot->setData(targets[iProperty], plotTrajectories, activePlots);
    plot->setXLabel(allTrajectories[0][iProperty].propertyUserName() + " [" + units[iProperty] + "]" );
    for (const QString& surfaceLine : surfaceLines.keys())
    {
      plot->drawSurfaceLine(surfaceLine, surfaceLines[surfaceLine]);
    }
    plots_.push_back(plot);
  }
}

QHBoxLayout* MultiWellPlot::obtainPlotSelectionAndLegendLayout()
{
  QHBoxLayout* plotSelectionAndLegendLayout = new QHBoxLayout();
  plotSelectionAndLegendLayout->addWidget(plotSelection_, Qt::AlignLeft);
  if (plots_.size() > 0)
  {
    legend_ = new Legend(plots_[0]->plotSettings(), plots_[0]->plotDataForLegend(), plots_[0]->legend(), this);
    plotSelectionAndLegendLayout->addWidget(legend_);
    plotSelectionAndLegendLayout->setStretch(1,1);
  }
  else
  {
    plotSelectionAndLegendLayout->addSpacerItem(new QSpacerItem(0,0));
    plotSelectionAndLegendLayout->setStretch(1,1);
  }
  plotSelectionAndLegendLayout->setStretch(0,0);

  return plotSelectionAndLegendLayout;
}


void MultiWellPlot::setExpanded(const bool isExpanded, const int plotID)
{
  expandedPlots_[plotID] = isExpanded;
}

} // namespace sac

} // namespace casaWizard
