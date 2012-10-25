#include "gridmapview.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qlineedit.h>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>

#include "rasterplot.h"

#ifdef NOSCROLLEDZOOM
#include "qwt_plot_zoomer.h"
#else
#include "scrollzoomer.h"
#endif

#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Reservoir.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/InputValue.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

using namespace DataAccess;
using namespace Interface;

#include "array.h"
/* 
 *  Constructs a GridMapView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
GridMapView::GridMapView( QWidget* parent,  const char* name, WFlags fl )
    : GridMapViewBase( parent, name, fl ), m_gridMap (0)
{
   m_mapView->setNumColors (64);
   m_mapView->setAxisMaxMajor (QwtPlot::xBottom, 8);
   m_mapView->setAxisMaxMajor (QwtPlot::xTop, 8);
   m_mapView->setAxisMaxMajor (QwtPlot::yLeft, 12);
   m_mapView->setAxisMaxMajor (QwtPlot::yRight, 12);

   m_mapViewLegend->setNumColors (64);
   m_mapViewLegend->setAxisMaxMajor (QwtPlot::yRight, 12);

   setupZooming ();
   setupMouseTracking ();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
GridMapView::~GridMapView()
{
    // no need to delete child widgets, Qt does it all for us
}

void GridMapView::setupZooming ()
{
#ifdef NOSCROLLEDZOOM
   m_zoomBL = new QwtPlotZoomer (m_mapView->canvas());
#else
   m_zoomBL = new ScrollZoomer(m_mapView->canvas());
   m_zoomBL->setHScrollBarPosition (ScrollZoomer::AttachedToScale);
   m_zoomBL->setVScrollBarPosition (ScrollZoomer::AttachedToScale);
#endif
   m_zoomBL->setAxis (QwtPlot::xBottom, QwtPlot::yLeft);
   m_zoomBL->setRubberBandPen(Qt::red);
   m_zoomBL->setEnabled (true);

}

void GridMapView::setGridMap( const GridMap * gm )
{
   QApplication::setOverrideCursor (QCursor::WaitCursor);

   m_gridMap = gm;
   m_mapView->clear ();
   m_mapViewLegend->clear ();

   if (!m_gridMap)
   {
      m_mapViewLegend->replot ();
      m_mapView->replot ();

      QApplication::restoreOverrideCursor ();
      return;
   }

   const Grid *g = m_gridMap->getGrid ();

   double min;
   double max;

   m_gridMap->getMinMaxValue (min, max);
   if (min == max)
   {
      min *= 0.9;
      max *= 1.1;
      if (min > max)
      {
	 double tmp;
	 tmp = min;
	 min = max;
	 max = tmp;
      }
      if (min == max)
      {
	 min -= 5; max += 5;
      }
   }

   unsigned int numColors = m_mapViewLegend->getNumColors ();
   m_mapViewLegend->setGrid (2, numColors + 1, 0, min, 1, max, m_gridMap->getUndefinedValue ());

   double **mvlValues = Array<double>::create2d (2, numColors + 1);
   for (int k = 0; k < numColors + 1; ++k)
   {
      mvlValues[0][k] = mvlValues[1][k] = min + (max - min) * k / (numColors);
   }

   m_mapViewLegend->setValues (mvlValues);
   Array<double>::delete2d (mvlValues);

   double const * const * const * gmValues = m_gridMap->getValues ();
   if (!gmValues)
   {
      cerr << "no gm values" << endl;
      QApplication::restoreOverrideCursor ();
      return;
   }

   m_mapView->setGrid (g->numI (), g->numJ (),
	 g->minI(), g->minJ (), g->maxI (), g->maxJ (), m_gridMap->getUndefinedValue ());

   double ** values = Array<double>::create2d (g->numI (), g->numJ ());
   for (int i = 0; i < g->numI (); ++i)
   {
      for (int j = 0; j < g->numJ (); ++j)
      {
          values[i][j] = gmValues[i][j][0];
      }
   }

   m_mapView->setValues (values);
   Array<double>::delete2d (values);

   m_zoomBL->setZoomBase ();
   m_zoomBL->zoom (0);

   m_mapViewLegend->setAxisMargins (QwtPlot::yRight, 0, 0);
   m_mapViewLegend->replot ();
   m_mapView->replot ();

   QApplication::restoreOverrideCursor ();
}

void GridMapView::showTitle ()
{
   m_mapViewTitle->show ();
}

void GridMapView::hideTitle ()
{
   m_mapViewTitle->hide ();
}

void GridMapView::setTitle (const QString & title)
{
   m_mapViewTitle->setText (title);
}

void GridMapView::displayPoint (const QwtDoublePoint & point)
{
   if (!m_gridMap) return;

   QString strX = "%1";
   QString strY = "%1";
   QString strValue;

   int x = int (point.x () + 0.5);
   if (x < 0) x = 0;
   if (x > m_mapView->getNumX () - 1) x = m_mapView->getNumX () - 1;
   strX = strX.arg (x);

   int y = int (point.y () + 0.5);
   if (y < 0) y = 0;
   if (y > m_mapView->getNumY () - 1) y = m_mapView->getNumY () - 1;
   strY = strY.arg (y);

   m_lineGridX->setText (strX);
   m_lineGridY->setText (strY);

   strX = "%1";
   strY = "%1";

   const Grid *g = m_gridMap->getGrid ();

   double posX, posY;
   g->getPosition (point.x (), point.y (), posX, posY);

   strX = strX.arg (posX);
   strY = strY.arg (posY);

   m_lineWorldX->setText (strX);
   m_lineWorldY->setText (strY);

   double value = m_mapView->getValue (x, y);
   if (value != m_mapView->getUndefinedValue ())
   {
      strValue.setNum (value);
   }
   else
   {
      strValue = "undefined";
   }
   m_lineValue->setText (strValue);
}

void GridMapView::setupMouseTracking ()
{
   m_plotPicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPicker::PointSelection | QwtPicker::DragSelection, 
        QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOff, 
        m_mapView->canvas());
   m_plotPicker->setMouseTracking (true);

   connect (m_plotPicker, SIGNAL (moved (const QwtDoublePoint &)),
	 this, SLOT (displayPoint (const QwtDoublePoint &)));
}

