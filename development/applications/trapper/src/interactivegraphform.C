/// Class InteractiveGraphForm inherits from GraphForm
/// and it allows the user to enable/disable graph lines
/// and zoom into the plot

#include "interactivegraphform.h"

#include <qlabel.h>

#include <qwt_plot.h>
#include <qwt_curve.h>
#include <qwt_plot_classes.h>

InteractiveGraphForm::InteractiveGraphForm
      (QWidget* parent, const char* name, WFlags fl)
   : GraphForm (parent, name, fl)
{
   connect (plot(), SIGNAL (legendClicked(long)), SLOT (toggleCurve(long)));
   updateStatusBar ("Click on a legend to enable/disable it's property");
}

//
// PROTECTED
//
/// InteractiveGraphForm::toggleCurve turns the visibility of a graph line on or off
/// and also makes the legend name bold or plain respectively
void InteractiveGraphForm::toggleCurve(long curveKey)
{
   QwtPlotCurve *c = m_plot->curve (curveKey);
   c->setEnabled (! c->enabled() );
   setLegendBold (c->enabled(), curveKey);
   m_plot->replot (); 
}

/// InteractiveGraphForm::zoom calls the base class zoom
/// and also uses the status bar for its own comment when the 
/// zoom is switched off
void InteractiveGraphForm::zoom(bool zoomOn)
{
   GraphForm::zoom (zoomOn);
   
   if ( ! zoomOn )
   {
      updateStatusBar ("Click on a legend to enable/disable it's property");
   }
}
