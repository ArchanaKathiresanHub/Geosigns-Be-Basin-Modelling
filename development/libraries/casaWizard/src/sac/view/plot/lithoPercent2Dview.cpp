//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithoPercent2Dview.h"

#include "../colormap.h"
#include "../lithoMapsToolTip.h"
#include "model/optimizedLithofraction.h"
#include "model/input/cmbDataAccess.h"

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

LithoPercent2DView::LithoPercent2DView(const ColorMap& colormap, QWidget* parent) :
  WellBirdsView(colormap, parent),
  lithoMapsToolTip_{new LithoMapsToolTip(this)},
  wellsVisible_{true}
{
  lithoMapsToolTip_->setVisible(false);
}

void LithoPercent2DView::setWellsVisible(const bool wellsVisible)
{
  wellsVisible_ = wellsVisible;
  update();
}

void LithoPercent2DView::drawData(QPainter& painter)
{
  Grid2DView::drawData(painter);

  if (wellsVisible_)
  {
    painter.save();
    drawPieChartsWells(painter);
    drawPieChartsSelectedWells(painter);
    painter.restore();
  }
}

void LithoPercent2DView::drawPieChartsWells(QPainter& painter)
{
  const int shade = selectedWells().empty() ? 255 : 150;
  painter.setPen(Qt::black);
  painter.setRenderHint(QPainter::Antialiasing, true);
  int counter = 0;
  for (const OptimizedLithofraction& optimizedLitho : optimizedLithofractions_)
  {
    drawPieChart(painter, shade, 10, counter, optimizedLitho);
    counter++;
  }
}

void LithoPercent2DView::drawPieChart(QPainter& painter, const int shade, const int size, const int counter, const OptimizedLithofraction& optimizedLitho)
{
  QPoint position = valToPoint(x()[counter], y()[counter]).toPoint();

  double firstAngle = - optimizedLitho.optimizedPercentageFirstComponent()/100*360*16;
  double secondAngle = - optimizedLitho.optimizedPercentageSecondComponent()/100*360*16;
  double thirdAngle = - optimizedLitho.optimizedPercentageThirdComponent()/100*360*16;

  painter.setBrush(QColor(shade, 0, 0));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16, firstAngle);

  painter.setBrush(QColor(0, shade, 0));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16+ firstAngle, secondAngle);

  painter.setBrush(QColor(0, 0, shade));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16+secondAngle + firstAngle, thirdAngle);
}


void LithoPercent2DView::drawPieChartsSelectedWells(QPainter& painter)
{
  if (optimizedLithofractions_.empty())
  {
    return;
  }

  QPen border(Qt::black);
  border.setWidthF(1.5);
  painter.setPen(border);
  painter.setRenderHint(QPainter::Antialiasing, true);

  for (int i : selectedWells())
  {
    drawPieChart(painter, 255, 12, i, optimizedLithofractions_[i]);
  }
}

void LithoPercent2DView::setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions)
{
  optimizedLithofractions_ = optimizedLithofractions;
  update();
}

void LithoPercent2DView::mousePressEvent(QMouseEvent* event)
{
  setToolTipVisible(false);
  QPoint mousePosition = event->pos();

  if (validPosition(mousePosition.x(), mousePosition.y()))
  {
    initializeToolTip(mousePosition);
  }
}

void LithoPercent2DView::initializeToolTip(const QPoint& mousePosition)
{
  const QPointF domainPosition = pointToVal(mousePosition.x(), mousePosition.y());
  lithoMapsToolTip_->setDomainPosition(domainPosition);

  lithoMapsToolTip_->move(mousePosition);
  lithoMapsToolTip_->setMaximumHeight(height()/2);

  emit toolTipCreated(domainPosition);
}

void LithoPercent2DView::setToolTipData(const std::vector<double>& lithofractionsAtPoint, const int activePlot)
{
  lithoMapsToolTip_->setLithofractions(lithofractionsAtPoint, activePlot);
}

void LithoPercent2DView::correctToolTipPositioning()
{
  QPoint toolTipPosition = lithoMapsToolTip_->pos();
  const bool moveX = !validPosition(toolTipPosition.x() + lithoMapsToolTip_->width(), toolTipPosition.y());
  const bool moveY = !validPosition(toolTipPosition.x(), toolTipPosition.y() + lithoMapsToolTip_->height());
  if (moveY)
  {
    lithoMapsToolTip_->move(toolTipPosition - (QPoint(0,lithoMapsToolTip_->height())));
    toolTipPosition = lithoMapsToolTip_->pos();
  }
  if (moveX)
  {
    lithoMapsToolTip_->move(toolTipPosition - (QPoint(lithoMapsToolTip_->width(),0)));
  }
  lithoMapsToolTip_->setCorner(moveX, moveY);
  lithoMapsToolTip_->setVisible(true);
}

void LithoPercent2DView::setToolTipVisible(const bool visible)
{
  lithoMapsToolTip_->setVisible(visible);
}

void LithoPercent2DView::setToolTipLithotypes(const QStringList& lithotypes)
{
  lithoMapsToolTip_->setLithoNames(lithotypes);
}

} // namespace sac

} // namespace casaWizard
