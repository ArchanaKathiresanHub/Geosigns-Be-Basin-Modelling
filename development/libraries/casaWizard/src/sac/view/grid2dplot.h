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

class ColorBar;
class ColorMap;
class LithoPercent2DView;

class Grid2DPlot : public QWidget
{
  Q_OBJECT
public:
  explicit Grid2DPlot(const ColorMap& colormap, QWidget *parent = nullptr);
  LithoPercent2DView* lithoPercent2DView() const;
  void setTitle(const QString& lithoType, int id, bool inputMap);
  void updateColorBar();
  void hideColorBar();
  void showColorBar();
  void refresh();

private:
  ColorBar* colorBar_;
  LithoPercent2DView* lithoPercent2dView_;
  CustomTitle* title_;

  void setTitleLayout();
};


} // namespace sac

} // namespace casaWizard
