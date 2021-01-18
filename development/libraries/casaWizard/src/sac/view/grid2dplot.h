//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class CustomTitle;

namespace casaWizard
{

namespace sac
{

class Grid2DView;
class ColorBar;
class ColorMap;

class Grid2DPlot : public QWidget
{
  Q_OBJECT
public:
  explicit Grid2DPlot(const ColorMap& colormap, QWidget *parent = nullptr);
  Grid2DView* grid2DView() const;
  void setTitle(const QString& lithoType, int id);
  void updateColorBar();

private:
  ColorBar* colorBar_;
  Grid2DView* grid2dView_;
  CustomTitle* title_;

  void setTitleLayout();
};


} // namespace sac

} // namespace casaWizard
