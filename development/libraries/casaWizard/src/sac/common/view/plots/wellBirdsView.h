//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Birds eye view for the wells, with the option to select those
#pragma once

#include "view/plots/grid2Dview.h"

namespace casaWizard
{

namespace sac
{

class WellBirdsView : public Grid2DView
{
   Q_OBJECT

public:
   explicit WellBirdsView(const ColorMap& colormap, QWidget* parent = nullptr);

   void setWellLocations(const QVector<double>& x, const QVector<double>& y);
   void setSelectedWells(const QVector<int> selectedWells);
   const QVector<int>& selectedWells();
   const QVector<double>& x() const;
   const QVector<double>& y() const;

   void drawData(QPainter& painter) override;

private:
   QVector<double> m_x;
   QVector<double> m_y;
   QVector<int> m_selectedWells;

   QVector<double> convertToKm(const QVector<double>& distancesInMeter);
};

}  // namespace sac

}  // namespace casaWizard
