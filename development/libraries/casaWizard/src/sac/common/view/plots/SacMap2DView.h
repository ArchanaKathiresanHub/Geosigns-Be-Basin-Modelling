//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "view/plots/wellBirdsView.h"

#include "vector"

namespace casaWizard
{

class ColorMap;

namespace sac
{

class SacMapToolTip;

class SacMap2DView : public WellBirdsView
{
  Q_OBJECT

public:
  explicit SacMap2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void correctToolTipPositioning();

  void setToolTipVisible(const bool visible);
  void setWellsVisible(const bool wellsVisible);
  void moveTooltipToDomainLocation(const QPointF& domainLocation);

  void mousePressEvent(QMouseEvent* event) override;

protected:
  virtual SacMapToolTip* mapToolTip() = 0;
  bool wellsVisible();

private:
  bool m_wellsVisible;

  void initializeToolTip(const QPoint& mousePosition);

signals:
  void toolTipCreated(const QPointF&);
};

} // namespace sac

} // namespace casaWizard
