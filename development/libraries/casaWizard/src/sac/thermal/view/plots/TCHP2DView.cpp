//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TCHP2DView.h"

#include "view/colormap.h"
#include "view/assets/TCHPMapToolTip.h"
#include "model/OptimizedTCHP.h"
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

namespace thermal
{

TCHP2DView::TCHP2DView(const ColorMap& colormap, QWidget* parent) :
   SacMap2DView(colormap, parent),
   m_tchpMapToolTip{new TCHPMapToolTip(this)}
{
   m_tchpMapToolTip->setVisible(false);
}

void TCHP2DView::finalizeTooltip(const double& TCHPAtPoint, const QString& wellName)
{
   setToolTipVisible(true);
   setToolTipData(TCHPAtPoint, wellName);
   correctToolTipPositioning();
}

void TCHP2DView::drawData(QPainter& painter)
{
   Grid2DView::drawData(painter);

   if (wellsVisible())
   {
      painter.save();
      drawPointsWells(painter);
      drawPointsSelectedWells(painter);
      painter.restore();
   }
}

TCHPMapToolTip*TCHP2DView::mapToolTip()
{
   return m_tchpMapToolTip;
}

void TCHP2DView::drawPointsWells(QPainter& painter)
{
   painter.setRenderHint(QPainter::Antialiasing, true);
   for(int counter = 0; counter < m_optimizedTCHPs.size(); ++counter)
   {
      drawPoint(painter, 8, counter, m_optimizedTCHPs[counter]);
   }
}

void TCHP2DView::drawPoint(QPainter& painter, const int size, const int counter, const OptimizedTCHP& optTCHP)
{
   QPoint position = valToPoint(x()[counter], y()[counter]).toPoint();
   int borderWidth = 1;

   painter.setPen(QPen(QBrush(colorMap().getContrastingColor()), size));
   painter.drawEllipse(QPoint(position), (size/2 + borderWidth), (size/2 + borderWidth));

   painter.setPen(QPen(QBrush(colorMap().getColor(optTCHP.optimizedHeatProduction(), m_range->first, m_range->second)), size));
   painter.drawEllipse(QPoint(position), size/2, size/2);
}


void TCHP2DView::drawPointsSelectedWells(QPainter& painter)
{
   if (m_optimizedTCHPs.empty())
   {
      return;
   }

   painter.setRenderHint(QPainter::Antialiasing, true);

   for (int i : selectedWells())
   {
      drawPoint(painter, 12, i, m_optimizedTCHPs[i]);
   }
}

void TCHP2DView::setOptimizedTCHPs(const QVector<OptimizedTCHP>& optimizedTCHPs)
{
   m_optimizedTCHPs = optimizedTCHPs;
   determineRange();
   update();
}

void TCHP2DView::determineRange()
{
   Grid2DView::determineRange();
   double min = m_range->first;
   double max = m_range->second;

   for( auto& tchp: m_optimizedTCHPs)
   {
      double val = tchp.optimizedHeatProduction();
      if (val < min)
      {
         min = val;
      }
      if (val > max)
      {
         max = val;
      }
   }

   m_range.reset(new std::pair<double, double>(min, max));
}

void TCHP2DView::setToolTipData(const double& TCHPAtPoint, const QString& wellName)
{
   m_tchpMapToolTip->setTCHP(TCHPAtPoint, wellName);
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
