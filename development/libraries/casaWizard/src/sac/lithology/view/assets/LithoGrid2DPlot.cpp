//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithoGrid2DPlot.h"

#include "view/sharedComponents/customtitle.h"
#include "view/plots/lithoPercent2Dview.h"

namespace casaWizard
{

namespace sac
{

LithoGrid2DPlot::LithoGrid2DPlot(const ColorMap& colormap, QWidget *parent) :
   Grid2DPlot(colormap, parent),
   m_lithoPercent2dView(new LithoPercent2DView(colormap, this))
{
   initGrid2DPlot();
}

LithoPercent2DView* LithoGrid2DPlot::get2DView() const
{
   return m_lithoPercent2dView;
}

void LithoGrid2DPlot::setTitle(const QString& lithoType, int id, bool inputMap)
{
   title()->setText(QString(inputMap ? "INPUT MAP - " : "") + "Lithotype " + QString::number(id + 1) + ": " + lithoType);
}

} // namespace sac

} // namespace casaWizard
