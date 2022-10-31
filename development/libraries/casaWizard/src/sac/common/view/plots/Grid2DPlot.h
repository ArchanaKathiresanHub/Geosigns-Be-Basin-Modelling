//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class CustomTitle;
class QGridLayout;

namespace casaWizard
{

class ColorMap;
class ColorBar;

namespace sac
{

class WellBirdsView;

class Grid2DPlot : public QWidget
{
  Q_OBJECT
public:
  explicit Grid2DPlot(const ColorMap& colormap, QWidget *parent = nullptr);
  virtual WellBirdsView* get2DView() const = 0;
  CustomTitle* title() const;
  void setTitle(const QString& title);
  void updateColorBar(const int& maxDecimals = 2);
  void setColorBarMap(const ColorMap& map);
  void hideColorBar();
  void showColorBar();
  void refresh();

protected:
  void initGrid2DPlot();

private:
  ColorBar* m_colorBar;
  CustomTitle* m_title;
  QGridLayout* m_layout;

  void setTitleLayout();
};


} // namespace sac

} // namespace casaWizard
