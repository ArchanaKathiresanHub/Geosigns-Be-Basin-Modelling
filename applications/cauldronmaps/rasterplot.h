#ifndef RASTERPLOT_H
#define RASTERPLOT_H

#include <qwt_plot.h>
class QBrush;
class QwtDoubleRect;

class RasterPlot : public QwtPlot 
{
   Q_OBJECT
public:

   RasterPlot(QWidget * p = 0, const char * name = 0, unsigned int numColors = 0);
   ~RasterPlot (void);

   bool setGrid (unsigned int numX, unsigned int numY,
	 double minX, double minY, double maxX, double maxY, double undefined);

   bool setValues (double ** values);
   bool setBrushes (void);

   void setNumColors (unsigned int numColors);
   inline unsigned int getNumColors (void) const;

   inline unsigned int getUndefinedIndex (void) const;

   double getValue (unsigned int x, unsigned int y);

   inline unsigned int getNumX (void);
   inline unsigned int getNumY (void);

   inline double getUndefinedValue (void);

   virtual void setAxisScale(int axis, double min, double max, double step = 0);

public slots:
   void clear ();
   virtual void replot ();

protected:
   virtual void drawCanvas(QPainter *painter);
   virtual void drawCanvasItems(QPainter *, const QRect &,
	 const QwtArray<QwtDiMap> &, const QwtPlotPrintFilter &) const;

private slots:

private:
   mutable int m_callDepth;
   mutable int m_maxCallDepth;
   unsigned int m_numX;
   unsigned int m_numY;
   double m_minX;
   double m_minY;
   double m_maxX;
   double m_maxY;

   double m_undefinedValue;

   int ** m_coordsX;
   int ** m_coordsY;
   double ** m_values;
   QBrush * m_brushes;
   unsigned int ** m_brushIndices;

   unsigned int m_numColors;

   double m_maxValue;
   double m_minValue;

   unsigned int m_mainIndex;

};

unsigned int RasterPlot::getNumX (void)
{
   return m_numX;
}

unsigned int RasterPlot::getNumY (void)
{
   return m_numY;
}

double RasterPlot::getUndefinedValue (void)
{
   return m_undefinedValue;
}

unsigned int RasterPlot::getNumColors (void) const
{
   return m_numColors;
}

unsigned int RasterPlot::getUndefinedIndex (void) const
{
   return m_numColors + 1;
}

#endif // RASTERPLOT_H
