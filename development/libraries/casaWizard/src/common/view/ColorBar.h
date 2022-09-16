//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once

#include "colormap.h"
#include <QWidget>

namespace casaWizard {

class ColorBar: public QWidget
{
   Q_OBJECT

public:
   enum orientation {
      left = 0,
      top = 1,
      right = 2,
      bottom = 3
   };

   ColorBar( orientation = orientation::left, QWidget * = nullptr );

   virtual void setOrientation( orientation );
   void setRange( const double& min, const double &max, const int &numberOfIntervals = 10);
   void setColorMap( const ColorMap& colormap );
   void setTitle( const QString& title );
   void setFont( const QFont& font );
   void setDecimals( const int& decimals );

   ColorMap colorMap() const { return m_colorMap; }
   orientation orientation() const { return m_orientation; }
   QString title() const {return m_title; }
   QFont font() const {return m_font; }

Q_SIGNALS:
   void selected( const QColor & );

protected:
   virtual void paintEvent( QPaintEvent * );

private:
   enum orientation m_orientation;
   ColorMap m_colorMap;
   QString m_title;
   QFont m_font;

   int m_heigth;
   int m_width;
   int m_Yoffset;
   int m_Xoffset;
   int m_textSpacing;
   int m_maxDecimals;
   double m_rangeMin;
   double m_rangeMax;
   double m_intervals;

   QString getTickLabel(const int& tick) const;
   void getColorBarDimentions();
   void drawTicksAndLabels( QPainter * ) const;
   void drawTicksAndLabelsLeftSide( QPainter * ) const;
   void drawTicksAndLabelsRightSide( QPainter * ) const;
   void drawColorBar( QPainter * ) const;
};

} //casaWizard
