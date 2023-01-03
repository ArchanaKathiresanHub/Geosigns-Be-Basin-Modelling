//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithoPercent2Dview.h"

#include "view/colormap.h"
#include "view/assets/lithoMapsToolTip.h"
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

namespace lithology
{

LithoPercent2DView::LithoPercent2DView(const ColorMap& colormap, QWidget* parent) :
  SacMap2DView(colormap, parent),
  m_lithoMapsToolTip{new LithoMapsToolTip(this)}
{
  m_lithoMapsToolTip->setVisible(false);
}

void LithoPercent2DView::finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID)
{
  setToolTipVisible(true);
  setToolTipData(lithofractionsAtPoint, wellName, plotID);
  correctToolTipPositioning();
}

void LithoPercent2DView::drawData(QPainter& painter)
{
  Grid2DView::drawData(painter);

  if (wellsVisible())
  {
    painter.save();
    drawPieChartsWells(painter);
    drawPieChartsSelectedWells(painter);
    painter.restore();
  }
}

LithoMapsToolTip* LithoPercent2DView::mapToolTip()
{
   return m_lithoMapsToolTip;
}

void LithoPercent2DView::drawPieChartsWells(QPainter& painter)
{
  const int shade = selectedWells().empty() ? 255 : 150;
  painter.setPen(Qt::black);
  painter.setRenderHint(QPainter::Antialiasing, true);
  int counter = 0;
  for (const OptimizedLithofraction& optimizedLitho : m_optimizedLithofractions)
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
  if (m_optimizedLithofractions.empty())
  {
    return;
  }

  QPen border(Qt::black);
  border.setWidthF(1.5);
  painter.setPen(border);
  painter.setRenderHint(QPainter::Antialiasing, true);

  for (int i : selectedWells())
  {
    drawPieChart(painter, 255, 12, i, m_optimizedLithofractions[i]);
  }
}

void LithoPercent2DView::setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions)
{
  m_optimizedLithofractions = optimizedLithofractions;
  update();
}

void LithoPercent2DView::setToolTipData(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int activePlot)
{
   m_lithoMapsToolTip->setLithofractions(lithofractionsAtPoint, wellName, activePlot);
}

void LithoPercent2DView::setToolTipLithotypes(const QStringList& lithotypes)
{
  m_lithoMapsToolTip->setLithoNames(lithotypes);
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
