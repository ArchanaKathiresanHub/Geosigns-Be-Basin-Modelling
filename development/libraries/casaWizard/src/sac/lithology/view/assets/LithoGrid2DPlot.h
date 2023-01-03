//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>
#include "view/plots/Grid2DPlot.h"
#include "view/plots/lithoPercent2Dview.h"

class CustomTitle;

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithoGrid2DPlot : public Grid2DPlot
{
  Q_OBJECT
public:
  explicit LithoGrid2DPlot(const ColorMap& colormap, QWidget *parent = nullptr);

  LithoPercent2DView* get2DView() const final;
  void setTitle(const QString& lithoType, int id, bool inputMap);

private:
  LithoPercent2DView* m_lithoPercent2dView;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
