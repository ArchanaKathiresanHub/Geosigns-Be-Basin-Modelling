//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalGrid2DPlot.h"
#include "TCHP2DView.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalGrid2DPlot::ThermalGrid2DPlot(const ColorMap& colormap, QWidget *parent) :
   Grid2DPlot(colormap, parent),
   m_TCHP2DView(new TCHP2DView(colormap, this))
{
   initGrid2DPlot();
}

TCHP2DView* ThermalGrid2DPlot::get2DView() const
{
   return m_TCHP2DView;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
