/// Class RightAxisGraphForm inherits from InteractiveGraphForm
/// adding functionality to enable the right axis of the graph

#include "rightaxisgraphform.h"

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>

#include "globalstrings.h"
using namespace Graph_Properties;

//
// PUBLIC METHODS
//
/// RightAxisGraphForm::RightAxisGraphForm initialises right axis attributes
RightAxisGraphForm::RightAxisGraphForm
      (QWidget* parent, const char* name, WFlags fl)
   : InteractiveGraphForm (parent, name, fl)
{
   m_zoomRight = 0;
   prevAutoScaleRightY = true;
}

/// RightAxisGraphForm::setRightYAxisLabel enables the right axis label
void RightAxisGraphForm::setRightYAxisLabel (const string& label)
{
   addRightYAxis (m_plot);
   setAxisLabel (m_plot->yRight, label);
}

void RightAxisGraphForm::setRightYAxisScale (double min, double max, int inc)
{
    m_plot->setAxisScale (m_plot->yRight, min, max, inc);
    prevAutoScaleRightY = false;
}

void RightAxisGraphForm::setRightYAxisAutoScale ()
{
    m_plot->setAxisAutoScale (m_plot->yRight);
    prevAutoScaleRightY = true;
}

//
// PUBLIC SLOTS
//
/// RightAxisGraphForm::zoom switches zoom on and off on the right axis
void RightAxisGraphForm::zoom(bool zoomOn)
{
   m_zoomRight->setEnabled (zoomOn);
   m_zoomRight->zoom (0);
   
   InteractiveGraphForm::zoom (zoomOn);
}

//
// PROTECTED METHODS
//
/// RightAxisGraphForm::handleQwtZoomBug handles a QT bug which involves
/// loosing autoscaling after a zoom operation, if the autoscaling was previously switched on
void RightAxisGraphForm::handleQwtZoomBug (bool zoomOn)
{
   // zoom seems to switch off auto scaling so if its on befor
   // zoom then must switch it back on afterwards
   if ( zoomOn )
   {
      // record status of auto scale before zoom
      prevAutoScaleRightY = m_plot->axisAutoScale (m_plot->yRight);       
   }
   else
   {
      // if auto scale was set previously then set back again 
      if ( prevAutoScaleRightY ) setRightYAxisAutoScale (); 
   }
   
   InteractiveGraphForm::handleQwtZoomBug (zoomOn);
}

//
// PRIVATE METHODS
//
void RightAxisGraphForm::setPointersToNull()
{
   m_zoomRight = 0;
   InteractiveGraphForm::setPointersToNull();
}

/// RightAxisGraphForm::addCurve attaches a graph line to the right axis
long RightAxisGraphForm::addCurve (const char *curveName, QwtPlot *plot)
{
   long curveId = InteractiveGraphForm::addCurve (curveName, plot);

   // set up curve axis
   if ( strcmp (curveName, VolumeGas) == 0 || 
        strcmp (curveName, VolumeOil) == 0 ||
        strcmp (curveName, TrapCapacity) == 0 )
   {
      plot->setCurveYAxis (curveId, QwtPlot::yRight);
   }
   else
   {
      plot->setCurveYAxis (curveId, QwtPlot::yLeft);
   }
   
   // disable curve by default
   toggleCurve(curveId);

   return curveId;
}

void RightAxisGraphForm::addRightYAxis (QwtPlot *plot)
{
   plot->enableYRightAxis (true);
}

void RightAxisGraphForm::createNewZoom ()
{
   m_zoomRight = new QwtPlotZoomer (QwtPlot::xTop, QwtPlot::yRight,
                                  QwtPicker::DragSelection, QwtPicker::AlwaysOff, 
                                  m_plot->canvas());
   m_zoomRight ->setRubberBand(QwtPicker::NoRubberBand);
   
   InteractiveGraphForm::createNewZoom ();
}

void RightAxisGraphForm::deleteOldZoom()
{
   if ( m_zoomRight ) 
   {
      delete m_zoomRight;
      m_zoomRight = 0;
   }
   
   InteractiveGraphForm::deleteOldZoom ();
}


