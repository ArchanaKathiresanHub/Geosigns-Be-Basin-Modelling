#include "multiWellPlot.h"

#include "model/calibrationTarget.h"
#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

#include "view/plot/wellPlot.h"

#include <QGridLayout>
#include <QLabel>

namespace casaWizard
{

namespace sac
{

MultiWellPlot::MultiWellPlot(QWidget* parent) :
  QWidget(parent),
  plots_{}
{
  QGridLayout* layout = new QGridLayout();
  layout->addWidget(new QLabel("No well selected", this), 0, 0);

  setLayout(layout);
}

void MultiWellPlot::updatePlots(const QVector<QVector<CalibrationTarget>> targets,
                                const QStringList& properties,
                                const QVector<QVector<WellTrajectory>> allTrajectories,
                                const QVector<bool>& activePlots)
{
  for (WellPlot* plot : plots_)
  {
    delete plot;
  }
  plots_.clear();

  QGridLayout* total = static_cast<QGridLayout*>(layout());

  const int nPlots = targets.size();
  const int nTypes = allTrajectories.size();
  for (int iPlot = 0; iPlot < nPlots; ++iPlot)
  {
    WellPlot* plot = new WellPlot(this);
    QVector<WellTrajectory> plotTrajectories(nTypes, {});
    for (int i = 0; i < nTypes; i++)
    {
      plotTrajectories[i] = allTrajectories[i][iPlot];
    }

    plot->setData(targets[iPlot], plotTrajectories, activePlots);
    plot->setXLabel(allTrajectories[0][iPlot].propertyUserName());
    plots_.push_back(plot);
    total->addWidget(plot, 0, iPlot);
  }  
}

} // namespace sac

} // namespace casaWizard
