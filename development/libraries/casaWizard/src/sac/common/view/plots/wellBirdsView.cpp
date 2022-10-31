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
  m_x{},
  m_y{}
{
  setXLabel("x [km]");
  setYLabel("y [km]");
}

void WellBirdsView::setWellLocations(const QVector<double>& x, const QVector<double>& y)
{
  Plot::clearData();
  m_x = convertToKm(x);
  m_y = convertToKm(y);
  addXYscatter(m_x, m_y);
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
  m_selectedWells = selectedWells;
  Plot::clearData();
  if (selectedWells.size() == 0)
  {
    addXYscatter(m_x, m_y);
    update();
    return;
  }

  QVector<double> xActive, yActive, xInactive, yInactive;

  for (int i = 0; i< m_x.size(); ++i)
  {
    if (selectedWells.contains(i))
    {
      xActive.append(m_x[i]);
      yActive.append(m_y[i]);
    }
    xInactive.append(m_x[i]);
    yInactive.append(m_y[i]);
  }

  addXYscatter(xInactive, yInactive);
  addXYscatter(xActive, yActive, SymbolType::Circle, 0);
  update();
}

const QVector<int>& WellBirdsView::selectedWells()
{
  return m_selectedWells;
}

const QVector<double>& WellBirdsView::x() const
{
  return m_x;
}

void WellBirdsView::drawData(QPainter& painter)
{
  Grid2DView::drawData(painter);
  Plot::drawData(painter);
}

const QVector<double>&WellBirdsView::y() const
{
  return m_y;
}

}  // namespace sac

}  // namespace casaWizard
