//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapsVisualisation.h"
#include "MapPlotOptions.h"

#include "view/sharedComponents/customcheckbox.h"

namespace casaWizard
{

namespace sac
{

MapsVisualisation::MapsVisualisation(QWidget *parent) :
   QWidget(parent),
   colormap_{new ColorMap}
{}

ColorMap& MapsVisualisation::colormap()
{
   return *colormap_;
}

const ColorMap& MapsVisualisation::colormap() const
{
   return *colormap_;
}

QCheckBox* MapsVisualisation::wellsVisible() const
{
   return plotOptions()->wellsVisible();
}

} // namespace sac

} // namespace casaWizard

