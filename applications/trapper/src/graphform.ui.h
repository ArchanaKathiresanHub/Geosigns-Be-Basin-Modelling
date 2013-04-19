//
// Base Graph Form
// Loads DataReader data into plot and displays plot
//

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qcanvas.h>
#include <qmessagebox.h>

#include <string>
#include <map>
#include "includestreams.h"
#include "datareader.h"
#include "environmentsettings.h"
#include "lineattributes.h"
#include "usersettings.h"
#include "globalstrings.h"
using namespace Graph_Properties;

// typedefs
typedef map<string, long> CurveRefs;

//
// CTOR /DTOR (init/destroy)
//

void GraphForm::init()
{
   setPointersToNull ();
   setGraphFonts ();   
   m_plot = createNewPlot ();
   
   // set up signal and slot so clicking on graph will show graph coordinates
   setupGraphXYLocation ();
   
   // set up zoom botton to enable/disable zoom
   setupZoomButton();
   
   createMessageBox ();
   
   m_plot->show ();
   
   m_userSettings = 0;
   
   // set auto scaling true by default
   prevAutoScaleX = true;
   prevAutoScaleY = true;
}

void GraphForm::destroy (bool destroyWindows, bool destroySubWindows)
{
   // not using but adding here to avoid warning that its being hidden by the function below
}

void GraphForm::destroy() 
{
   deregisterGraph();
   
   // NOT DELETING THESE CAUSE THEY
   // ARE DELETED AUTOMATICALLY..
   // delete m_plot;
   // delete m_picker;
   // deleteOldZoom ();
   
   if ( m_reader ) delete m_reader;
   if ( m_printer ) delete m_printer;
}

//
// PUBLIC METHODS
//
void GraphForm::setReader(DataReader *reader)
{ 
   m_reader = reader;
}

DataReader* GraphForm::getReader()
{
   return m_reader;
}

void GraphForm::setCurrentUserSettings (const UserSettings *settings)
{
   m_userSettings = settings;
}

const UserSettings* GraphForm::getCurrentUserSettings ()
{
   return m_userSettings;
}

string GraphForm::loadPlotData ()
{ 
   DataReader *reader = getReader ();
      
   // retrieve vector of x,y arrays from reader and add to plot
   const ReservoirLineGroup *resLineGroup = reader->getXYVector ();
      
   if ( resLineGroup->size() == 0 ) 
   {
      return "Error. No Data Found.";
   }
      
   string plotErrorMessage;
   const TrapLineGroup *trapLines;
   const LineGroup *lineGroup;
   const_ReservoirLineGroupIT resIt, endReservoirs = resLineGroup->end();
   const_TrapLineGroupIT trapIt, endTraps;
   const_LineGroupIT lineIt, endLines;
   
   long curveId = 0;
  
   // loop reservoirs
   for ( resIt = resLineGroup->begin(); resIt != endReservoirs; ++resIt )
   {
      // loop traps
      trapLines = &(resIt->second);
      endTraps = trapLines->end();
      for ( trapIt = trapLines->begin(); trapIt != endTraps; ++trapIt )
      {
         // loop properties per trap 
         lineGroup = &(trapIt->second);
         
         endLines = lineGroup->end();
         for ( lineIt = lineGroup->begin(); lineIt != endLines; ++lineIt )
         {  
            // if only one age then skip as cannot be plotted
            if ( (lineIt->second).used() < 2 ) 
            {
               plotErrorMessage += "Data for ";
               plotErrorMessage += lineIt->first.name;
               plotErrorMessage += " only exists at one timestep\n";
             }
            
            if ( EnvironmentSettings::debugOn() )
            {
               printDebug ((lineIt->first).c_str(), (lineIt->second).getXPoints(), 
                           (lineIt->second).getYPoints(), (lineIt->second).used());
            }
            
            // add curve to plot or get curve already created
            curveId = getCurveRef ((lineIt->first).c_str());
            if ( curveId == -1 )
            {         
               curveId = addCurve ((lineIt->first).c_str(), m_plot);
            }
            
            // add data to curve
            m_plot->setCurveRawData (curveId, (lineIt->second).getXPoints(), 
                                     (lineIt->second).getYPoints(), 
                                     (lineIt->second).used());
         }
      }
   }

   return plotErrorMessage;
}

void GraphForm::updatePlot()
{
   // need to call replot before and after zoom because
   // of internal qwt bug
   m_plot->replot ();
   refreshZoom ();
   m_plot->replot ();
}

void GraphForm::clearPlotData ()
{
   m_plot->clear ();
   clearCurveRefs ();
}

void GraphForm::hide()
{
   if ( isEnabled () )
   {
      setEnabled (false);
      closeAndDelete ();
   }
}

void GraphForm::setHeading(const string &title, const string &reservoir, const string &age, const string &trap)
{
   char heading [1000];
   sprintf(heading, "%s  -  %s,  Age: %s,  Trap: %s", 
           title.c_str(), reservoir.c_str(), age.c_str(), trap.c_str());
         
   setCaption (heading);
}

void GraphForm::closeGraph()
{
   hide ();
}

void GraphForm::setXAxisLabel (const string& label)
{
   setAxisLabel (m_plot->gridXAxis (), label);
}

void GraphForm::setYAxisLabel (const string& label)
{
   setAxisLabel (m_plot->gridYAxis (), label);
}

QwtPlot* GraphForm::createNewPlot()
{
   QwtPlot *plot = new QwtPlot (this);
   
   plot->move (30, 20);
   
   plot->setAutoLegend (TRUE);
   plot->setLegendFont (m_normalGraphFont);
   
   return plot;
}

void GraphForm::setLeftYAxisScale (double min, double max, int inc)
{
   m_plot->setAxisScale (m_plot->gridYAxis (), min, max, inc);
   prevAutoScaleY = false;
}

void GraphForm::setXAxisScale (double min, double max, int inc)
{
   m_plot->setAxisScale (m_plot->gridXAxis (), min, max, inc);
   prevAutoScaleX = false;
}

void GraphForm::setXAxisAutoScale ()
{
   m_plot->setAxisAutoScale (m_plot->gridXAxis ());
   prevAutoScaleX = true;
}

void GraphForm::setYAxisAutoScale ()
{
   m_plot->setAxisAutoScale (m_plot->gridYAxis ());
   prevAutoScaleY = true;
}

void GraphForm::updateStatusBar (const char *status)
{
   statusLabel->setText (status);
}

void GraphForm::setLegendBold (bool boldOn, long legendId)
{
   QwtLegend *l = m_plot->legend ();
   QWidget *w = l->findItem (legendId);
   
   if ( w )
   {
      QFont f = w->font ();
      f.setBold (boldOn);
      w->setFont (f);
   }
}

void GraphForm::setupGraphXYLocation()
{
   m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, 
                                QwtPicker::PointSelection, // | QwtPicker::DragSelection, 
                                QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, 
                                m_plot->canvas());
}

string GraphForm::refreshGraph()
{
   // redraw graph
   string error = loadPlotData ();
 
   updatePlot ();
   resizePlot ();
 
   return error;
}

//
// PROTECTED METHODS
//
void GraphForm::setPointersToNull()
{
   m_reader = 0;
   m_printer = 0;
   m_zoom = 0;
}

void GraphForm::deregisterGraph()
{
   GraphFactory *graphFactory = GraphFactory::getInstance ();
   graphFactory->removeGraph (this);
}
      
//
// PRIVATE METHODS
//

void GraphForm::closeAndDelete(void)
{
   close ();
   this->~GraphForm();
}

void GraphForm::setAxisLabel (int axis, const string &label)
{
   // set title
   QString title = label.c_str(); 
   m_plot->setAxisTitle (axis, title);
   
   // set font
   m_plot->setAxisTitleFont (axis, m_boldGraphFont);
}

long GraphForm::addCurve (const char *curveName, QwtPlot *plot)
{
   // create and add new curve to plot
   long curveId = plot->insertCurve (curveName);
   
   // get curve attributes
   const LineAttributes *lineAttribs = getLineAttributes (curveName);
   
   if ( lineAttribs ) 
   {
      // set curve with pre-decided line attributes
      plot->setCurvePen (curveId, QPen (lineAttribs->colour, lineAttribs->width));
   }
   else 
   {
      // create default line attributes
      LineAttributes la (getGeneralColour (), 2);
      plot->setCurvePen (curveId, QPen (la.colour, la.width));
   }
   
   // set legend pen
   setLegendBold (true, curveId);
   
   // add curve name and id to curve refs
   addCurveRef (curveName, curveId);
   
   return curveId;
}

const LineAttributes* GraphForm::getLineAttributes (const char *lineName)
{
   LineAttributesList *list = LineAttributesList::getList ();
   return list->getLineAttributes (lineName);
}

void GraphForm::componentSelectedSlot()
{
   // have to put this slot in here so can be recognised by derived class
   // SourceGraphForm
}

void GraphForm::toggleCurve(long curveKey)
{
   // have to put this slot in here so can be recognised by derived class
   // InteractiveGraphForm. 
   //
   // am just assigning key here to stop warning.
   curveKey = curveKey;
}

void GraphForm::setGraphFonts ()
{
   m_normalGraphFont = QFont ("Lucida Sans", 10);
   m_boldGraphFont = m_normalGraphFont;
   m_boldGraphFont.setBold (true);
}

void GraphForm::resizeEvent (QResizeEvent *size)
{
   QWidget::resizeEvent(size);
   resizePlot ();
}

void GraphForm::resizePlot()
{
   m_plot->resize (width() - 70, height() - 80);
}

void GraphForm::printSlot()
{
   if ( ! m_printer )
   {
      m_printer = new QPrinter ();
   }
   
   if( m_printer->setup (this) )
   {      
      m_plot->print (*m_printer);
   } 
}

QwtPlot* GraphForm::plot()
{
   return m_plot;
}

const QColor& GraphForm::getGeneralColour ()
{
   LineAttributesList *list = LineAttributesList::getList ();
   return list->getGeneralColour ();
}

void GraphForm::resetGeneralColourIndex ()
{
   LineAttributesList::getList ()->resetGeneralColourIndex ();
}

void GraphForm::setupZoomButton()
{
   zoomButton->setToggleButton (true);
   connect(zoomButton, SIGNAL(toggled(bool)), SLOT(zoom(bool)));
}

void GraphForm::refreshZoom()
{
   deleteOldZoom ();
   createNewZoom ();
   
   zoom (false);
}

void GraphForm::createNewZoom()
{
   m_zoom = new QwtPlotZoomer (QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPicker::DragSelection, QwtPicker::AlwaysOff, 
                               m_plot->canvas());
}

void GraphForm::zoom(bool zoomOn)
{   
   m_zoom->setEnabled (zoomOn);
   m_zoom->zoom (0);
   
   m_picker->setRubberBand(
         zoomOn ? QwtPicker::NoRubberBand : QwtPicker::CrossRubberBand);     
   
   if ( zoomOn )
   {
      handleQwtZoomBug (zoomOn);      
      updateStatusBar ("Press right button and drag mouse to create zoom window");
      zoomButton->setPixmap (QPixmap::fromMimeSource( "zoomout.png" ));
   }
   else
   {
      updateStatusBar ("");
      zoomButton->setPixmap (QPixmap::fromMimeSource( "zoomin.png" ));           
      handleQwtZoomBug (zoomOn);
   }
   
   m_plot->replot ();
}

void GraphForm::handleQwtZoomBug (bool zoomOn)
{
   // zoom seems to switch off auto scaling so if its on befor
   // zoom then must switch it back on afterwards
   
   if ( zoomOn )
   {
      // record status of auto scale before zoom
      prevAutoScaleX = m_plot->axisAutoScale (m_plot->gridXAxis ());
      prevAutoScaleY = m_plot->axisAutoScale (m_plot->gridYAxis());      
   }
   else
   {
      // if auto scale was set previously then set back again
      if ( prevAutoScaleX ) setXAxisAutoScale ();     
      if ( prevAutoScaleY ) setYAxisAutoScale (); 
   }
}

void GraphForm::deleteOldZoom()
{ 
   if ( m_zoom ) 
   {
      delete m_zoom;
      m_zoom = 0;
   }
}

void GraphForm::printDebug (const char *lineName, const double *x, const double *y, int size)
{
   cout << endl << "Graph line : " << lineName << endl;
   for ( int i=0; i < size; ++i )
   {
      cout << "(" << x[i] << ", " << y[i] << ")" << endl;
   }
   cout << endl;
}

void GraphForm::showMessage(const char *str)
{
   m_msg->setText (str);
   m_msg->exec();
}

void GraphForm::clearCurveRefs()
{
   m_curveRefs.erase (m_curveRefs.begin(), m_curveRefs.end());
}

long GraphForm::getCurveRef(const char * name) 
{
   CurveRefs::const_iterator it = m_curveRefs.find (name);   
   return (it == m_curveRefs.end()) ? -1 : it->second;
}

void GraphForm::addCurveRef(const char *name, long id)
{
   m_curveRefs.insert (pair<string, long> (name, id));
}

void GraphForm::createMessageBox()
{
   m_msg = new QMessageBox ("TrapTracking Warning", "Warning", 
                            QMessageBox::Warning, 1, 0, 0, this);
   
   QString buttonText = "Okay";
   m_msg->setButtonText (0, buttonText);
}
