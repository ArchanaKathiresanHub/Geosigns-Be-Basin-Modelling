//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMap2DView.h"

#include "view/colormap.h"
#include "model/input/cmbDataAccess.h"
#include "view/assets/SacMapToolTip.h"

// QT
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

#include <cmath>


namespace casaWizard
{

namespace sac
{

SacMap2DView::SacMap2DView(const ColorMap& colormap, QWidget* parent) :
  WellBirdsView(colormap, parent),
  m_wellsVisible{true}
{
}

void SacMap2DView::setWellsVisible(const bool wellsVisible)
{
  m_wellsVisible = wellsVisible;
  setToolTipVisible(false);

  update();
}

void SacMap2DView::moveTooltipToDomainLocation(const QPointF& domainLocation)
{
  mapToolTip()->setDomainPosition(domainLocation);
  mapToolTip()->move(valToPoint(domainLocation.x(),domainLocation.y()).toPoint());
}


void SacMap2DView::mousePressEvent(QMouseEvent* event)
{
  setToolTipVisible(false);
  QPoint mousePosition = event->pos();

  if (validPosition(mousePosition.x(), mousePosition.y()))
  {
    initializeToolTip(mousePosition);
  }
}

bool SacMap2DView::wellsVisible()
{
   return m_wellsVisible;
}

void SacMap2DView::initializeToolTip(const QPoint& mousePosition)
{
  const QPointF domainPosition = pointToVal(mousePosition.x(), mousePosition.y());
  mapToolTip()->setDomainPosition(domainPosition);
  mapToolTip()->move(mousePosition);
  mapToolTip()->setMaximumHeight(height()/2);

  emit toolTipCreated(domainPosition);
}

void SacMap2DView::correctToolTipPositioning()
{
  QPoint toolTipPosition = mapToolTip()->pos();
  const bool moveX = !validPosition(toolTipPosition.x() + mapToolTip()->width(), toolTipPosition.y());
  const bool moveY = !validPosition(toolTipPosition.x(), toolTipPosition.y() + mapToolTip()->height());
  if (moveY)
  {
    mapToolTip()->move(toolTipPosition - (QPoint(0,mapToolTip()->height())));
    toolTipPosition = mapToolTip()->pos();
  }
  if (moveX)
  {
    mapToolTip()->move(toolTipPosition - (QPoint(mapToolTip()->width(),0)));
  }
  mapToolTip()->setCorner(moveX, moveY);
  mapToolTip()->setVisible(true);
}

void SacMap2DView::setToolTipVisible(const bool visible)
{
  mapToolTip()->setVisible(visible);
}

} // namespace sac

} // namespace casaWizard
