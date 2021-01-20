#include "wellBirdsView.h"

#include "ConstantsMathematics.h"
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
  setXLabel("x [km]");
  setYLabel("y [km]");
}

void WellBirdsView::setWellLocations(const QVector<double>& x, const QVector<double>& y)
{
  clearData();
  x_ = convertToKm(x);
  y_ = convertToKm(y);
  addXYscatter(x_, y_);
  update();
}

QVector<double> WellBirdsView::convertToKm(const QVector<double>& distancesInMeter)
{
  QVector<double> distancesInKm;
  for (const double distance : distancesInMeter)
  {
    distancesInKm.push_back(distance * Utilities::Maths::MeterToKilometer);
  }

  return distancesInKm;
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