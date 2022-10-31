//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapsVisualisation.h"
#include "SacMapPlotOptions.h"

#include "view/sharedComponents/customcheckbox.h"

namespace casaWizard
{

namespace sac
{

SacMapsVisualisation::SacMapsVisualisation(QWidget *parent) :
   QWidget(parent),
   colormap_{new ColorMap}
{}

ColorMap& SacMapsVisualisation::colormap()
{
   return *colormap_;
}

const ColorMap& SacMapsVisualisation::colormap() const
{
   return *colormap_;
}

QCheckBox* SacMapsVisualisation::wellsVisible() const
{
   return plotOptions()->wellsVisible();
}

} // namespace sac

} // namespace casaWizard

