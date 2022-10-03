//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellBirdsView.h"

#include "ConstantsMathematics.h"

namespace casaWizard
{

namespace sac
{

WellBirdsView::WellBirdsView(const ColorMap& colormap, QWidget* parent) :
  Grid2DView(colormap, parent),
  x_{},
  y_{}
{
  setXLabel("x [km]");
  setYLabel("y [km]");
}

void WellBirdsView::setWellLocations(const QVector<double>& x, const QVector<double>& y)
{
  Plot::clearData();
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
  Plot::clearData();
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

void WellBirdsView::drawData(QPainter& painter)
{
  Grid2DView::drawData(painter);
  Plot::drawData(painter);
}

const QVector<double>&WellBirdsView::y() const
{
  return y_;
}

}  // namespace sac

}  // namespace casaWizard
