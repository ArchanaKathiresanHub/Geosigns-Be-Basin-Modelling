#include "wellBirdsView.h"

#include <assert.h>

namespace casaWizard
{

namespace sac
{

WellBirdsView::WellBirdsView(QWidget* parent) :
  Plot(parent),
  x_{},
  y_{}
{
  setMaximumSize(400, 250);
  setXLabel("x [m]");
  setYLabel("y [m]");
}

void WellBirdsView::setWellLocations(const QVector<double> x, const QVector<double> y)
{
  clearData();
  addXYscatter(x, y);
  x_ = x;
  y_ = y;
  update();
}

void WellBirdsView::setActiveWells(const QVector<int> activeWells)
{
  clearData();
  if (activeWells.size() == 0)
  {
    addXYscatter(x_, y_);
    update();
    return;
  }

  QVector<double> xActive, yActive, xInactive, yInactive;

  for (int i = 0; i< x_.size(); ++i)
  {
    if (activeWells.contains(i))
    {
      xActive.append(x_[i]);
      yActive.append(y_[i]);
    }
    else
    {
      xInactive.append(x_[i]);
      yInactive.append(y_[i]);
    }
  }

  addXYscatter(xInactive, yInactive);
  addXYscatter(xActive, yActive, SymbolType::Circle, 0);
  update();
}

}  // namespace sac

}  // namespace casaWizard
