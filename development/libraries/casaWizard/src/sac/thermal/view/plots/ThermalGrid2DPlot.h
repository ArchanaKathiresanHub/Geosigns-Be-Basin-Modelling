//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>
#include "view/plots/Grid2DPlot.h"
#include "TCHP2DView.h"

class CustomTitle;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalGrid2DPlot : public Grid2DPlot
{
  Q_OBJECT

public:
  explicit ThermalGrid2DPlot(const ColorMap& colormap, QWidget *parent = nullptr);
  TCHP2DView* get2DView() const final;

private:
  TCHP2DView* m_TCHP2DView;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
