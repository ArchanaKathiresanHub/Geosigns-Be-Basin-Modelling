/// Class TrapTrackingKernel initiates all functionaliy requested by the user
/// through the user interface

#include "traptrackingkernel.h"
#include "traptracking.h"
#include "datareader.h"
#include "graphform.h"
#include "readonlydatabase.h"
#include "environmentsettings.h"
#include "usersettings.h"
#include "axisRange.h"
#include "propertiesreader.h"
#include "sourcereader.h"
#include "componentsreader.h"
#include "componentsreaderpvt.h"
#include "interactivegraphform.h"
#include "componentgraphform.h"
#include "sourcegraphform.h"
#include "rightaxisgraphform.h"
#include "simplesmartpointer.h"
#include "graphfactory.h"
#include "currentgraphs.h"

//
// PUBLIC METHODS
//
TrapTrackingKernel::TrapTrackingKernel ()
   : m_graphFactory (GraphFactory::getInstance()),
   m_currGraphs (CurrentGraphs::getInstance())
{}

void TrapTrackingKernel::setGui (TrapTrackingForm *gui)
{
   m_gui = gui;
}

void TrapTrackingKernel::openDatabase (const char* projectName)
{
   ReadOnlyDatabase *dbase = ReadOnlyDatabase::getReadOnlyDatabase (); 
   dbase->openDatabase (projectName);
}

/// TrapTrackingKernel::displayGraph sets a graphs heading and calls its show function
string TrapTrackingKernel::displayGraph (GraphForm *graph, const string title)
{ 
   // load plot data from data reader
   string error = graph->refreshGraph(); 

   char userAge [20];
   sprintf(userAge, "%2.2f", m_gui->getUserAge());
   graph->setHeading (title, m_gui->getReservoirNameStr(), userAge, 
                      m_gui->getUserTrapIdStr());
   graph->show ();

   return error;
}

/// TrapTrackingKernel::generatePersisTraps calls the PersisTraps object's generate function
/// to generate the trap history. It also collects and displas errors if they occur
string TrapTrackingKernel::generatePersisTraps (void)
{
   string error;
 
   m_persisTraps.clearPersisTraps ();

   if ( EnvironmentSettings::debugOn() )
   {
      m_persisTraps.createPersisTrapOutputFile (m_gui->getProjectNameStr ().c_str());
   }
   
   m_persisTraps.generatePersisTrapOutput 
         (m_gui->getProjectNameStr(),
          m_gui->getOutputDirectoryNameStr());
  
   if ( ! m_persisTraps.trapGenerationOkay () )
   {
      error = m_persisTraps.errorDescription ();
   }

   if ( EnvironmentSettings::debugOn() )
   {
      m_persisTraps.savePersisTrapOutputFile ();
   }
  
   return error;
}

void TrapTrackingKernel::clearPersisTraps ()
{
   m_persisTraps.clearPersisTraps ();
}

void TrapTrackingKernel::convertUstoSpace (string &fileResName)
{
   m_persisTraps.convertUstoSpace (fileResName);
}

/// TrapTrackingKernel::readGraphData takes a reader type, initialise is user-request,
/// clears any previous data from it and calls its read functionality to re-read its table
string TrapTrackingKernel::readGraphData (DataReader *reader)
{
   // get user request
   string error = setupUserRequest (&m_userRequest);
   
   if ( error.size() == 0 ) 
   {
      // read data based on user request
      reader->setRequestFromUser (&m_userRequest);
      reader->clearData ();
      error = reader->readData (m_gui->getProjectNameStr().c_str());
   }
   
   return error;
}

/// TrapTrackingKernel::createPropertyGraph sets up a property graph, initilaising
/// the reader and graph ranges
string TrapTrackingKernel::createPropertyGraph ()
{ 
   // read in property data
   SimpleSmartPointer <PropertiesReader, PointerPolicy<PropertiesReader> > 
         propRead (new PropertiesReader ());
  
   string error = readGraphData (propRead.get()); 
       
   if ( error.size() == 0 ) 
   { 
      // create property graph
      RightAxisGraphForm *graph = m_graphFactory->createPropertyGraph ();
      
      // set reader
      graph->setReader (propRead.release());
            
      // set graph properties
      setupPropertyGraphProperties (graph);      
      setupPropertyGraphRanges (graph);
      
      // display
      error = displayGraph (graph, "Properties Graph");
   }
   
   return error;
}

/// TrapTrackingKernel::refreshPropertyGraph re-reads and re-displays a property graph
string TrapTrackingKernel::refreshPropertyGraph()
{ 
   string error;
   
   GraphForm *propertyGraph = m_currGraphs->getPropertyGraph();
   
   if ( propertyGraph )
   {
      error = readGraphData (propertyGraph->getReader());
      if ( error.size() == 0 )
      {
         error = displayGraph (propertyGraph, "Properties Graph");
      }
   }
   
   return error;
}

void TrapTrackingKernel::setupPropertyGraphProperties (RightAxisGraphForm *graph)
{
   // set axis labels
   graph->setXAxisLabel ("Age (Ma)");
   graph->setYAxisLabel ("Temperature (C)      Pressure (MPa)");
   graph->setRightYAxisLabel ("Volume (m3)");
}

/// TrapTrackingKernel::setupPropertyGraphRanges establishes the ranges for the property graph
/// based on the user settings selection
void TrapTrackingKernel::setupPropertyGraphRanges (RightAxisGraphForm *graph)
{
   UserSettings *settings = m_gui->userSettings ();
   if ( ! settings ) return;
   
   // set volume range
   if ( settings->oilGasVolumeRangeEnabled () )
   {
      AxisRange* volRange = settings->getOilGasVolRange();
      graph->setRightYAxisScale (volRange->getMin(), volRange->getMax(), 0);
   }
   
   // set pressure and temperature range
   if ( settings->tempPressRangeEnabled () )
   {
      AxisRange *pressTempRange = settings->getTempPressRange();
      graph->setLeftYAxisScale (pressTempRange->getMin(), pressTempRange->getMax(), 0);
   }
   
   // set age range
   if ( settings->snapshotRangeEnabled () )
   {
      AxisRange *ageRange = settings->getSnapshotRange ();
      graph->setXAxisScale (ageRange->getMin(), ageRange->getMax(), 0);
   }
}

/// TrapTrackingKernel::createComponentsGraph creates and displays a component graph
string TrapTrackingKernel::createComponentsGraph ()
{ 
   // create component reader
   SimpleSmartPointer <ComponentsReader, PointerPolicy<ComponentsReader> > 
         compRead (new ComponentsReader ());
  
   string error = readGraphData (compRead.get ()); 
   
   if ( error.size() == 0 ) 
   { 
      // create component graph
      ComponentGraphForm *graph = m_graphFactory->createComponentGraph();
      
      // set graph reader
      graph->setReader (compRead.release ());
      
      // set graph user settings as will be used later for conversions
      graph->setCurrentUserSettings (currentUserSettings ());
      
      // set graph properties
      setupComponnetGraphProperties (graph);
      setupComponentGraphRanges (graph);
     
      error = displayGraph (graph, "Components Graph");
   }
   
   return error;
}

/// TrapTrackingKernel::refreshComponentGraph re-reads and re-displays a component graph
string TrapTrackingKernel::refreshComponentGraph()
{
   string error;
   
   GraphForm *componentGraph = m_currGraphs->getComponentGraph ();
   if ( componentGraph )
   {
      error = readGraphData (componentGraph->getReader());
      if ( error.size() == 0 )
      {
         error = displayGraph (componentGraph, "Components Graph");
      }
   }
   
   return error;
}

void TrapTrackingKernel::setupComponnetGraphProperties (InteractiveGraphForm *graph)
{
   // set axis labels
   graph->setXAxisLabel ("Age (Ma)");
   graph->setYAxisLabel ("Mass (kg)");
}

/// TrapTrackingKernel::setupComponentGraphRanges sets component graph ranges based on user settings
void TrapTrackingKernel::setupComponentGraphRanges (InteractiveGraphForm *graph)
{
   UserSettings *settings = m_gui->userSettings ();
   if ( ! settings ) return;
   
   // set mass range
   if ( settings->massRangeEnabled () )
   {
      AxisRange* massRange = settings->getMassRange();
      graph->setLeftYAxisScale (massRange->getMin(), massRange->getMax(), 0);
   }
   
   // set age range
   if ( settings->snapshotRangeEnabled () )
   {
      AxisRange *ageRange = settings->getSnapshotRange ();
      graph->setXAxisScale (ageRange->getMin(), ageRange->getMax(), 0);
   }
}

/// TrapTrackingKernel::createSourceGraph creates a source graph
string TrapTrackingKernel::createSourceGraph ()
{
   // create source reader
   SimpleSmartPointer <SourceReader, PointerPolicy<SourceReader> > 
         srcRead (new SourceReader ());
  
   string error = readGraphData (srcRead.get ()); 
   
   if ( error.size() == 0 ) 
   {
      // create source graph
      SourceGraphForm *graph = m_graphFactory->createSourceGraph();
      
      // set graph reader
      graph->setReader (srcRead.release ());
      
      // set graph properties
      setupSourceGraphProperties (graph);
      setupSourceGraphRanges (graph);

      // display graph
      error = displayGraph (graph, "Source Rock Origins Graph");
   }
   
   return error;
}

/// TrapTrackingKernel::refreshSourceGraph refreshes and redisplays a source graph
string TrapTrackingKernel::refreshSourceGraph()
{
   string error;
   
   GraphForm *sourceGraph = m_currGraphs->getSourceGraph ();
   if ( sourceGraph )
   {
      error = readGraphData (sourceGraph->getReader());
      if ( error.size() == 0 )
      {
         error = displayGraph (sourceGraph, "Source Graph");
      }
   }
   
   return error;
}

void TrapTrackingKernel::setupSourceGraphProperties (SourceGraphForm *graph)
{
   // set axis labels
   graph->setXAxisLabel ("Age (Ma)");
   graph->setYAxisLabel ("Mass (kg)");
}

void TrapTrackingKernel::setupSourceGraphRanges (SourceGraphForm *graph)
{
   UserSettings *settings = m_gui->userSettings ();
   if ( ! settings ) return;
   
   // set mass range
   if ( settings->massRangeEnabled () )
   {
      AxisRange* massRange = settings->getMassRange();
      graph->setLeftYAxisScale (massRange->getMin(), massRange->getMax(), 0);
   }
   
   // set age range
   if ( settings->snapshotRangeEnabled() )
   {
      AxisRange *ageRange = settings->getSnapshotRange ();
      graph->setXAxisScale (ageRange->getMin(), ageRange->getMax(), 0);
   }
}

const UserSettings* TrapTrackingKernel::currentUserSettings ()
{
   return m_gui->userSettings ();
}

/// TrapTrackingKernel::checkProjectVersion will give the user a warning if the project file they are 
/// trying to open was created with an older version of Cauldron incompatable with trapper
bool TrapTrackingKernel::checkProjectVersion ()
{
   // open project table
   Table *t = (ReadOnlyDatabase::getReadOnlyDatabase ())->getTable ("ProjectIoTbl");
   if ( ! t ) return false;
   
   Record *rec = t->getRecord (0);
   string versionName = getProgramVersion (rec);
   float vNum = atoi ((versionName.substr(1)).c_str());
   
   // should be 2004.99 or later
   return vNum >= 2005 || versionName == "v2004.99";
}

int TrapTrackingKernel::getPersisId (const string& resName, const double age, const int transId) const
{
   return m_persisTraps.getPersisId (resName, age, transId);
}

int TrapTrackingKernel::getTransId (const string& resName, const double age, const int persisId) const
{
   return m_persisTraps.getTransId (resName, age, persisId);
}

//
// PRIVATE METHODS
//

/// TrapTrackingKernel::setupUserRequest creates a user request based on user selections
/// from the main screen
string TrapTrackingKernel::setupUserRequest(CreateUserRequest *userReq)
{
   string error;
   
   if ( m_gui->userRequestUpToDate () ) return error;
   
   userReq->clearUserRequest ();
   
   // build user request based on input data
   userReq->setUserRequestData (m_persisTraps.getPersistentTraps (), 
                                m_gui->getReservoirNameStr(),
                                m_gui->getUserAge(), 
                                m_gui->getUserTrapId());
   
   if ( userReq->userRequestEmpty () ) 
   {
      error = "No data found\n";
   }
   else
   {
      m_gui->setUserRequestUpToDate();
   }
   
   return error;
}


