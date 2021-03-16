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

void WellBirdsView::setSelectedWells(const QVector<int> selectedWells)
{
  selectedWells_ = selectedWells;
  clearData();
  if (selectedWells.size() == 0)
  {
    addXYscatter(x_, y_);
    update();
    return;
  }

  QVector<double> xActive, yActive, xInactive, yInactive;

  for (int i = 0; i< x_.size(); ++i)
  {
    if (selectedWells.contains(i))
    {
      xActive.append(x_[i]);
      yActive.append(y_[i]);
    }
    xInactive.append(x_[i]);
    yInactive.append(y_[i]);
  }

  addXYscatter(xInactive, yInactive);
  addXYscatter(xActive, yActive, SymbolType::Circle, 0);
  update();
}

const QVector<int>& WellBirdsView::selectedWells()
{
  return selectedWells_;
}

const QVector<double>& WellBirdsView::x() const
{
  return x_;
}

void WellBirdsView::updateRange(const double xMin, const double xMax, const double yMin, const double yMax)
{
  setMinMaxValues(xMin, xMax, yMin, yMax);
}

const QVector<double>&WellBirdsView::y() const
{
  return y_;
}

}  // namespace sac

}  // namespace casaWizard
