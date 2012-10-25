#include <assert.h>
#include <values.h>
#include <time.h>
#include <qdatetime.h>
#include <unistd.h>
#include <math.h>

#include <qeventloop.h>
#include <qpainter.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale.h>

#include "rasterplot.h"
#include "array.h"


int Round (double dx)
{
   return floor (dx + 0.5);
}

RasterPlot::RasterPlot(QWidget *parent, const char * name, unsigned int numColors):
    QwtPlot(parent, name), m_numX (0), m_numY (0), m_values (0),
    m_coordsX (0), m_coordsY (0), m_numColors (-1), m_brushes (0),
    m_callDepth (-1), m_maxCallDepth (-1)
{
   setNumColors (numColors);
}

RasterPlot::~RasterPlot (void)
{
   clear ();
   if (m_brushes)
   {
      Array<QBrush>::delete1d (m_brushes);
      m_brushes = 0;
   }
}

void RasterPlot::drawCanvas(QPainter *painter)
{
   QwtPlot::drawCanvas (painter);

}

// #define TIMING

void RasterPlot::drawCanvasItems(QPainter *painter, const QRect &rect,
        const QwtArray<QwtDiMap> &map, const QwtPlotPrintFilter &pfilter) const
{
   QRect window = painter->window ();

   if (m_numX != 0 && m_numY != 0 && m_values != 0)
   {
#ifdef TIMING
      QTime timer;
      timer.start ();
#endif

      const QwtDiMap &xMap = map[QwtPlot::xBottom];
      const QwtDiMap &yMap = map[QwtPlot::yLeft];

      double dl = xMap.invTransform (window.left ());
      double dr = xMap.invTransform (window.right ());
      double db = yMap.invTransform (window.bottom ());
      double dt = yMap.invTransform (window.top ());

      int il = int (dl);
      int ir = int (dr + 2);
      int ib = int (db);
      int it = int (dt + 2);

      if (il < 0) il = 0;
      if (ir > m_numX) ir = m_numX;
      if (ib < 0) ib = 0;
      if (it > m_numY) it = m_numY;

#ifdef TIMING
      cerr << "drawing: left = " << il
	 << ", right = " << ir
	 << ", bottom = " << ib
	 << ", top = " << it << endl;
      cerr << "# grid points = " << (ir - il) * (it - ib);
#endif

      int numDrawn = 0;

      QRect valueRect;

      unsigned int x;
      unsigned int y;

      for (x = il; x < ir; ++x)
      {
	 double left = double (x) - 0.5;
	 if (left < 0) left = 0;

	 double right = double (x) + 0.5;
	 if (right > m_numX - 1) right = m_numX - 1;

	 m_coordsX[x][0] = xMap.transform (left);
	 m_coordsX[x][1] = xMap.transform (right);
      }

      for (y = ib; y < it; ++y)
      {
	 double bottom = double (y) - 0.5;
	 if (bottom < 0) bottom = 0;

	 double top = double (y) + 0.5;
	 if (top > m_numY - 1) top = m_numY - 1;

	 m_coordsY[y][0] = yMap.transform (bottom);
	 m_coordsY[y][1] = yMap.transform (top);
      }

      int rectLeft = m_coordsX[il][0];
      int rectRight = m_coordsX[ir - 1][1];
      int rectBottom = m_coordsY[ib][0];
      int rectTop = m_coordsY[it - 1][1];

      painter->fillRect (rectLeft, rectBottom,
	    rectRight - rectLeft, rectTop - rectBottom,
	    m_brushes[m_mainIndex]);

      for (x = il; x < ir; ++x)
      {
	 rectLeft = m_coordsX[x][0];
	 rectRight = m_coordsX[x][1];
	 if (rectLeft == rectRight) continue;

         for (y = ib; y < it; ++y)
         {
	    rectTop = m_coordsY[y][1];
	    rectBottom = m_coordsY[y][0];

	    if (rectTop == rectBottom) continue;

	    unsigned int brushIndex = m_brushIndices[x][y];
	    assert (brushIndex < m_numColors + 2);

	    if (brushIndex == m_mainIndex) continue;

	    painter->fillRect (rectLeft, rectBottom,
		  rectRight - rectLeft, rectTop - rectBottom,
		  m_brushes[brushIndex]);
	    ++numDrawn;
         }
      }
#ifdef TIMING
      cerr << ", # drawn = " << numDrawn << endl;

      int duration = timer.elapsed ();
      cerr << "Finishing RasterPlot::drawCanvasItems\n";
      cerr << "Duration: " << duration << " milliseconds" << endl;
#endif
   }

   QwtPlot::drawCanvasItems (painter, rect, map, pfilter);
}

void RasterPlot::clear ()
{
   m_numX = 0;
   m_numY = 0;

   if (m_values )
   {
      Array<double>::delete2d (m_values);
      Array<unsigned int>::delete2d (m_brushIndices);
      Array<int>::delete2d (m_coordsX);
      Array<int>::delete2d (m_coordsY);
      m_values = 0;
      m_brushIndices = 0;
      m_coordsX = 0;
      m_coordsY = 0;
   }

   QwtPlot::clear ();
}
void RasterPlot::replot ()
{
   QwtPlot::replot ();
}

void RasterPlot::setAxisScale(int axis, double min, double max, double step)
{
   double minX, minY, maxX, maxY;

   switch (axis)
   {
      case QwtPlot::xBottom:
         minX = m_minX + (min / (m_numX - 1)) * (m_maxX - m_minX);
         maxX = m_minX + (max / (m_numX - 1)) * (m_maxX - m_minX);
         setAxisScale (QwtPlot::xTop, minX, maxX, step);
	 break;
      case QwtPlot::yLeft:
         minY = m_minY + (min / (m_numY - 1)) * (m_maxY - m_minY);
         maxY = m_minY + (max / (m_numY - 1)) * (m_maxY - m_minY);
         setAxisScale (QwtPlot::yRight, minY, maxY, step);
	 break;
      default:
	 break;
   }

   QwtPlot::setAxisScale (axis, min, max, step);
}

bool RasterPlot::setGrid (unsigned int numX, unsigned int numY,
      double minX, double minY, double maxX, double maxY, double undefined)
{
   if (m_values )
   {
      Array<double>::delete2d (m_values);
      Array<unsigned int>::delete2d (m_brushIndices);
      Array<int>::delete2d (m_coordsX);
      Array<int>::delete2d (m_coordsY);
   }

   m_numX = numX;
   m_numY = numY;
   m_minX = minX;
   m_minY = minY;
   m_maxX = maxX;
   m_maxY = maxY;

   m_undefinedValue = undefined;

   m_values = Array<double>::create2d (m_numX, m_numY, m_undefinedValue);
   m_brushIndices = Array<unsigned int>::create2d (m_numX, m_numY);
   m_coordsX = Array<int>::create2d (m_numX, 2, -1);
   m_coordsY = Array<int>::create2d (m_numY, 2, -1);

   setAxisScale (QwtPlot::xBottom, 0, m_numX - 1);
   setAxisScale (QwtPlot::yLeft, 0, m_numY - 1);

   setAxisScale (QwtPlot::xTop, m_minX, m_maxX);
   setAxisScale (QwtPlot::yRight, m_minY, m_maxY);

   return true;
}

void RasterPlot::setNumColors (unsigned int numColors)
{
   if (numColors == 0) numColors = 1;
   if (numColors == m_numColors) return;
   m_numColors = numColors;
   
   if (m_brushes)
   {
      Array<QBrush>::delete1d (m_brushes);
   }

   m_brushes = Array<QBrush>::create1d (m_numColors + 2);

   QColor tmpColor;

   int i;
   for (i = 0; i <= m_numColors; ++i)
   {
      double ratio = i / double (m_numColors);
      int hue = 300 - ratio * 300;
      int saturation = 255;
      int value = 255;

      tmpColor.setHsv (hue, saturation, value);
      m_brushes[i]= tmpColor;
   }

   // the 'undefinedValue' brush
   tmpColor.setRgb (150, 150, 150);
   m_brushes[getUndefinedIndex ()] = tmpColor;

   setBrushes ();
}

bool RasterPlot::setValues (double ** values)
{
   unsigned int x;
   unsigned int y;

   if (!m_values) return false;

   m_maxValue = -MAXDOUBLE;
   m_minValue = MAXDOUBLE;

   for (x = 0; x < m_numX; ++x)
   {
      for (y = 0; y < m_numY; ++y)
      {
         m_values[x][y] = values[x][y];

	 if (m_values[x][y] != getUndefinedValue ())
	 {
	    if (m_values[x][y] < m_minValue) m_minValue = m_values[x][y];
	    if (m_values[x][y] > m_maxValue) m_maxValue = m_values[x][y];
	 }
      }
   }

   setBrushes ();

   return true;
}

bool RasterPlot::setBrushes (void)
{
   if (m_numX == 0 || m_numY == 0 || m_values == 0) return false;

   unsigned int x;
   unsigned int y;

   QColor tmpColor;

   unsigned int * numOccurrences = Array<unsigned int>::create1d (m_numColors + 2, 0);

   for (x = 0; x < m_numX; ++x)
   {
      for (y = 0; y < m_numY; ++y)
      {
	 unsigned int brushIndex;
	 if (m_values[x][y] != getUndefinedValue ())
	 {
	    if (m_minValue != m_maxValue)
	    {
	       double range = m_maxValue - m_minValue;
	       double offset = m_values[x][y] - m_minValue;

	       double tmp = (offset / range) * m_numColors;
	       brushIndex = Round (tmp);
	       assert (brushIndex <= m_numColors);
	    }
	    else
	    {
	       brushIndex = Round (0.5 * m_numColors);
	    }

	 }
	 else
	 {
	    brushIndex = getUndefinedIndex ();
	 }

	 m_brushIndices[x][y]= brushIndex;
	 numOccurrences[brushIndex]++;
      }
   }

   int i;
   unsigned int maxNumOccurrences = numOccurrences[0];
   m_mainIndex = 0;
   
   for (i = 1; i < m_numColors + 2; ++i)
   {
      if (numOccurrences[i] > maxNumOccurrences)
      {
	 m_mainIndex = i;
	 maxNumOccurrences = numOccurrences[i];
      }
   }

   Array<unsigned int>::delete1d (numOccurrences);
   return true;
}

double RasterPlot::getValue (unsigned int x, unsigned int y)
{
   if (m_values && x < m_numX && y < m_numY)
   {
      return m_values[x][y];
   }
   else
   {
      return getUndefinedValue ();
   }
}
