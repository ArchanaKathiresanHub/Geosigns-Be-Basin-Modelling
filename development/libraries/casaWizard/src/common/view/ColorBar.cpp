//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ColorBar.h"
#include <qpainter.h>
#include <cmath>
#include "Qt_Utils.h"

namespace casaWizard {

ColorBar::ColorBar( enum orientation o, QWidget *parent ):
   QWidget( parent ),
   m_orientation( o ),
   m_colorMap(ColorMap(RAINBOW)),
   m_font({"Sans", 10}),
   m_textSpacing(3),
   m_maxDecimals(-1),
   m_rangeMin(0),
   m_rangeMax(100),
   m_intervals(10)
{
}

void ColorBar::setOrientation( enum orientation orientation )
{
   m_orientation = orientation;
   update();
}

void ColorBar::setColorMap(const ColorMap &map)
{
   m_colorMap = map;
   update();
}

void ColorBar::setTitle(const QString& title)
{
   m_title = title;
   update();
}

void ColorBar::setRange(const double &min, const double &max, const int &numberOfIntervals)
{
   m_rangeMin = min;
   m_rangeMax = max;
   m_intervals = numberOfIntervals;
   update();
}

void ColorBar::setDecimals(const int& d)
{
   d < 0 ? m_maxDecimals = -1 : m_maxDecimals = d;
   update();
}

void ColorBar::paintEvent( QPaintEvent * )
{
   QPainter painter( this );
   painter.setClipping(true);
   painter.setFont(m_font);
   getColorBarDimentions();
   if (m_orientation == orientation::bottom || m_orientation == orientation::top)
   {
      painter.translate(m_Xoffset + m_width/2.0, m_Yoffset + m_heigth/2.0); //move to middle
      painter.rotate(90); //rotate
      painter.translate(-m_Xoffset - m_width/2.0, -m_Yoffset - m_heigth/2.0); //move to right top corner
   }
   drawColorBar( &painter );
   drawTicksAndLabels( &painter );
}

void ColorBar::getColorBarDimentions()
{
   const QSize pixels = size();
   //define the how much space the colorbar will use of the Widget
   m_heigth = static_cast<int>(0.80*pixels.height());
   m_width = static_cast<int>(0.20*pixels.width());
   m_Yoffset = (pixels.height() - m_heigth) / 2;
   m_Xoffset = (pixels.width() - m_width) / 2;
}

QString ColorBar::getTickLabel(const int& tick) const
{
   return qtutils::doubleToQString(m_rangeMin + (std::abs(m_rangeMax - m_rangeMin) / double(m_intervals) * tick),
                                   m_maxDecimals);
}

void ColorBar::drawTicksAndLabelsLeftSide ( QPainter* painter ) const
{
   QFontMetrics fm(m_font);
   //Note: window/widget coordinate origin is in top left side
   QRect box; //recyclable boundingbox

   //Title
   painter->save();
   painter->translate(m_Xoffset + m_width + m_textSpacing, m_Yoffset + (m_heigth/2) + fm.width(m_title)/2);
   painter->rotate(-90);
   painter->drawText(QRect(), 0, m_title, &box); //defines boundingBox size
   painter->drawText(box, Qt::AlignCenter, m_title);
   painter->restore();

   //axis Line
   painter->fillRect(QRectF(m_Xoffset, m_Yoffset, 1, m_heigth),
                     QBrush("black"));

   //ticks & labels
   for (int tick = 0; tick <= m_intervals; tick++)
   {
      QString label = getTickLabel(tick);

      painter->save();

      painter->translate(m_Xoffset, m_Yoffset + (m_heigth - (m_heigth / double(m_intervals) * tick)));
      painter->fillRect(QRectF(0, 0, 10, 1), QBrush("black"));

      painter->translate(-fm.width(label) - m_textSpacing, -fm.height()/2);
      painter->drawText(QRect(), 0, label, &box); //defines boundingBox size
      painter->drawText(box, Qt::AlignRight, label);

      painter->restore();
   }
}

void ColorBar::drawTicksAndLabelsRightSide(QPainter *painter) const
{
   QFontMetrics fm(m_font);
   //Note: window/widget coordinate origin is in top left side
   QRect box; //recyclable boundingbox

   //Title
   painter->save();
   painter->translate(m_Xoffset - fm.height() - m_textSpacing, m_Yoffset + (m_heigth/2) + fm.width(m_title)/2);
   painter->rotate(-90);
   painter->drawText(QRect(), 0, m_title, &box); //defines boundingBox size
   painter->drawText(box, Qt::AlignCenter, m_title);
   painter->restore();

   //axis line
   painter->fillRect(QRectF(m_Xoffset + m_width, m_Yoffset, 1, m_heigth),
                     QBrush("black"));

   //ticks & labels
   for (int tick = 0; tick <= m_intervals; tick++)
   {
      QString label = getTickLabel(tick);

      painter->save();

      painter->translate(m_Xoffset + m_width, m_Yoffset + (m_heigth - (m_heigth / double(m_intervals) * tick)));
      painter->fillRect(QRectF(-10, 0, 10, 1), QBrush("black"));

      painter->translate(m_textSpacing, -fm.height()/2);
      painter->drawText(QRect(), 0, label, &box); //defines boundingBox size
      painter->drawText(box, Qt::AlignLeft, label);

      painter->restore();
   }
}

void ColorBar::drawTicksAndLabels( QPainter* painter ) const
{
   //Note: rotations for Top & bottom are being handled in paintEvent()
   if (m_orientation == orientation::top || m_orientation == orientation::left)
   {
      drawTicksAndLabelsLeftSide(painter);
   }
   else
   {
      drawTicksAndLabelsRightSide(painter);
   }
}

void ColorBar::drawColorBar( QPainter* painter ) const
{
   for (int i = 0; i <= m_heigth; i++)
   {
      painter->fillRect(QRectF(m_Xoffset, m_Yoffset + (m_heigth - i), m_width, 1),
                        QBrush(m_colorMap.getColor(i, 0, m_heigth)));
   }
}
} //casaWizard
