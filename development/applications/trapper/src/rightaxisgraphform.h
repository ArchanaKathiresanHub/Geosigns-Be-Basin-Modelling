/// Class RightAxisGraphForm inherits from InteractiveGraphForm
/// adding functionality to enable the right axis of the graph

#ifndef __propertygraphform__
#define __propertygraphform__

#include "interactivegraphform.h"

class QwtPlotZoomer;
class QwtPlot;

class RightAxisGraphForm : public InteractiveGraphForm
{
public:
   RightAxisGraphForm (QWidget* parent = 0, const char* name = 0, 
                       WFlags fl = 0);
   void setRightYAxisLabel (const string& label);
   void setRightYAxisScale (double min, double max, int inc);
   void setRightYAxisAutoScale ();
   
public slots:
   void zoom(bool zoomOn);
   
protected:
   void handleQwtZoomBug (bool zoomOn);
   
private:
   // private methods
   void setPointersToNull();
   void addRightYAxis (QwtPlot *plot);
   long addCurve (const char *curveName, QwtPlot *plot);
   void createNewZoom ();
   void deleteOldZoom();
   void updateZoomPicker ();
   
   // private variables
   QwtPlotZoomer *m_zoomRight;
   bool prevAutoScaleRightY; 
};

#endif
