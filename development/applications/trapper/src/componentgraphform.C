/// The Components Graph inherits from Interactive Graph
/// adding PVT calculation functionaliy and a graph options panel
/// so the user can switch between various graph calculations
/// causing PVT to be performed where necessary

#include "componentgraphform.h"
#include "componentsreaderpvt.h"
#include "createuserrequest.h"
#include "usersettings.h"
#include "axisRange.h"

#include <qwt_plot.h>
#include <qpushbutton.h>

//
// PUBLIC FUNCTIONS
//
/// ComponentGraphForm::setReader sets the current reader type and the user-request type from the reader
void ComponentGraphForm::setReader (ComponentsReader *reader)
{ 
   m_componentReader = reader; 
   m_userRequest = reader->userRequest ();
   setToComponentReader ();
}

/// ComponentGraphForm::updateGraph sets reader type to PVT
/// as currently all update options require a PVT calculation
/// It also collectes the user settingsselection for the update and then sets
/// the reader attributes according to the user selection
void ComponentGraphForm::updateGraph ()
{ 
   // get PVT reader as all graph options require pvt
   setToPvtReader ();
   
   // get user settings ptr
   const UserSettings *settings = getCurrentUserSettings ();
   
   // set conditions to stock tank or reservoir
   if ( strcmp (m_optionsFrame->conditionType(), StockTank) == 0 )
   {
      m_componentReaderPVT->setStockTankConditions ();
   }
   else
   {
      m_componentReaderPVT->setReservoirConditions ();
   }
      
   // set components to mass or volumes
   if ( strcmp (m_optionsFrame->componentType(), Masses) == 0 )
   {
      m_componentReaderPVT->setComputePvtMasses ();
      setYAxisLabel ("Mass (kg)");
      
      // impose range if it exists
      if ( settings && settings->massRangeEnabled () )
      {
         AxisRange* massRange = settings->getMassRange();
         setLeftYAxisScale (massRange->getMin(), massRange->getMax(), 0);
      }
      else
      {
         setYAxisAutoScale ();
      }
   }
   else
   {
      m_componentReaderPVT->setComputePvtVolumes ();
      setYAxisLabel (" Volumes (m3)");
      
      // impose range if exists
      if ( settings && settings->oilGasVolumeRangeEnabled () )
      {
         AxisRange* volRange = settings->getOilGasVolRange ();
         setLeftYAxisScale (volRange->getMin(), volRange->getMax(), 0);
      }
      else
      {
         setYAxisAutoScale (); 
      }
   }
   
   // set phase to oil, gas or oil and gas
   if ( strcmp (m_optionsFrame->phaseType (), OilPhase) == 0 )
   {
      m_componentReaderPVT->setOilPhase ();
   }
   else if ( strcmp (m_optionsFrame->phaseType (), GasPhase) == 0 )
   {
      m_componentReaderPVT->setGasPhase ();
   }
   else
   {
      m_componentReaderPVT->setTotalPhase ();
   }
   
   //  re-data with the correct reader
   rereadData ();
 
   // call refresh Graph to redisplay the newly read data
   refreshGraph ();
}

//
// PROTECTED METHODS
//
void ComponentGraphForm::createGraphOptions ()
{
   m_optionsFrame = new ComponentGraphOptionsFrame (this);
   m_optionsFrame->move (680, 10);
   m_optionsFrame->show ();
}

//
// PRIVATE METHODS
//

/// ComponentGraphForm::rereadData clears data
/// in the current reader and re-reads the necessary data
void ComponentGraphForm::rereadData()
{
   // re read table
   m_currentReader->clearData ();
   m_currentReader->readData ("TrapIoTbl");
}

/// ComponentGraphForm::resizePlot resizes the graph
/// and moves the options frame with it
void ComponentGraphForm::resizePlot ()
{
   plot()->move (10, 10);
   plot()->resize (width() - 280, height() - 70);
   m_optionsFrame->move (width() - 260, 10); 
}

/// ComponentGraphForm::getComponentReader returns the component reader
/// If the reader does not exist yet, it creats it
ComponentsReader* ComponentGraphForm::getComponentReader ()
{
   if ( ! m_componentReader )
   {
      m_componentReader = new ComponentsReader ();
      m_componentReader->setRequestFromUser (m_userRequest); 
   }
   
   return m_componentReader;
}

/// ComponentGraphForm::getPvtReader returns the component pvt reader
/// if the reader does not exist, it creates it
ComponentsReaderPVT* ComponentGraphForm::getPvtReader ()
{
   if ( ! m_componentReaderPVT )
   {
      m_componentReaderPVT = new ComponentsReaderPVT ();
      m_componentReaderPVT->setRequestFromUser (m_userRequest);
   }
   
   return m_componentReaderPVT;
}

void ComponentGraphForm::setToComponentReader ()
{
   m_currentReader = getComponentReader ();
}

void ComponentGraphForm::setToPvtReader ()
{
   m_currentReader = getPvtReader ();
}

void ComponentGraphForm::deleteReaders ()
{
   if ( m_componentReaderPVT) delete m_componentReaderPVT;
   if ( m_componentReader ) delete m_componentReader;
}

